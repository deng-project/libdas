// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: GLTFCompiler.cpp - GLTF format to DAS compiler implementation
// author: Karl-Mihkel Ott

#define GLTF_COMPILER_CPP
#include <GLTFCompiler.h>


namespace Libdas {

    GLTFCompiler::GLTFCompiler(const std::string &_out_file, bool _use_raw_textures) : 
        DasWriterCore(_out_file), m_use_raw_textures(_use_raw_textures) {}

    GLTFCompiler::GLTFCompiler(const GLTFRoot &_root, const DasProperties &_props, const std::string &_out_file, bool _use_raw_textures) : 
        m_use_raw_textures(_use_raw_textures)
    {
        Compile(_root, _props, _out_file);
    }


    GLTFCompiler::BufferAccessorData GLTFCompiler::_FindAccessorData(const GLTFRoot &_root, int32_t _accessor_id) {
        GLTFCompiler::BufferAccessorData accessor_data;
        accessor_data.buffer_id = static_cast<uint32_t>(_root.buffer_views[_root.accessors[_accessor_id].buffer_view].buffer);
        accessor_data.buffer_offset = static_cast<uint32_t>(_root.buffer_views[_root.accessors[_accessor_id].buffer_view].byte_offset +
                                                            _root.accessors[_accessor_id].byte_offset);

        return accessor_data;
    }


    void GLTFCompiler::_CheckAndSupplementProperties(GLTFRoot &_root, DasProperties &_props) {
        if(_props.author == "")
            _props.author = _root.asset.generator;
        if(_props.copyright == "")
            _props.copyright = _root.asset.copyright;

        // check if default scene exists
        if(_root.load_time_scene)
            _props.default_scene = static_cast<uint32_t>(_root.load_time_scene);
    }


    void GLTFCompiler::_FlagBuffersAccordingToMeshes(const GLTFRoot &_root, std::vector<DasBuffer> &_buffers) {
        // for each mesh in root
        for(auto it = _root.meshes.begin(); it != _root.meshes.end(); it++) {
            // for each primitive in mesh
            for(size_t i = 0; i < it->primitives.size(); i++) {
                int32_t size = static_cast<int32_t>(_buffers.size());
                // lol @ ur bugs with broken GLTF files
                if(it->primitives[i].indices != INT32_MAX)
                    _buffers[it->primitives[i].indices % size].type |= LIBDAS_BUFFER_TYPE_INDICES;

                // check into attributes
                for(auto map_it = it->primitives[i].attributes.begin(); map_it != it->primitives[i].attributes.end(); it++) {
                    std::string no_nr = String::RemoveNumbers(map_it->first);
                    // error check
                    if(m_attribute_type_map.find(no_nr) == m_attribute_type_map.end()) {
                        std::cerr << "GLTF error:  No valid attribute '" << map_it->first << "' available for current implementation" << std::endl;
                        std::exit(1);
                    }

                    const int32_t buffer_id = _root.buffer_views[_root.accessors[map_it->second].buffer_view].buffer;
                    _buffers[buffer_id % size].type |= m_attribute_type_map.find(no_nr)->second;
                }
            }
        }
    }


    void GLTFCompiler::_FlagBuffersAccordingToAnimations(const GLTFRoot &_root, std::vector<DasBuffer> &_buffers) {
        for(auto it = _root.animations.begin(); it != _root.animations.end(); it++) {
            for(size_t i = 0; i < it->samplers.size(); i++) {
                const int32_t input = _root.buffer_views[_root.accessors[it->samplers[i].input].buffer_view].buffer;
                const int32_t output = _root.buffer_views[_root.accessors[it->samplers[i].output].buffer_view].buffer;
                _buffers[input].type |= LIBDAS_BUFFER_TYPE_TIMESTAMPS;
                _buffers[output].type |= LIBDAS_BUFFER_TYPE_KEYFRAME;
            }
        }
    }


    std::vector<DasBuffer> GLTFCompiler::_CreateBuffers(const GLTFRoot &_root, const std::vector<std::string> &_embedded_textures) {
        std::vector<DasBuffer> buffers;

        m_buffers_size = _root.buffers.size();
        m_images_size = _root.images.size();

        // append buffers
        for(auto it = _root.buffers.begin(); it != _root.buffers.end(); it++) {
            DasBuffer buffer;
            buffer.type = LIBDAS_BUFFER_TYPE_UNKNOWN;
            buffer.data_len = it->byte_length;
            buffer.data_ptrs.push_back(std::make_pair<const char*, size_t>(reinterpret_cast<const char*>(_root.resources.at(it->uri).data()), buffer.data_len));

            buffers.push_back(buffer);
        }

        m_image_buffer_views.insert(m_image_buffer_views.begin(), _root.buffer_views.size(), INT32_MAX);

        // append images
        for(auto it = _root.images.begin(); it != _root.images.end(); it++) {
            // there are two possibilities:
            // 1. the image is defined with its uri
            // 2. the image is defined in some buffer view
            if(it->uri != "") {
                DasBuffer buffer;
                m_tex_readers.push_back(std::move(TextureReader(it->uri)));
                size_t len;

                buffer.data_len = _
                buffers.push_back(buffer);
            }
            else {
                BufferImageTypeResolver resolver;
                resolver.ResolveFromBufferView(it->mime_type);
                buffers[_root.buffer_views[it->buffer_view].buffer].type |= resolver.GetResolvedType();
            }
        }

        AppendTextures(buffers, _embedded_textures);
        _FlagBuffersAccordingToMeshes(_root, buffers);
        _FlagBuffersAccordingToAnimations(_root, buffers);

        return buffers;
    }


    std::vector<DasMesh> GLTFCompiler::_CreateMeshes(const GLTFRoot &_root) {
        std::vector<DasMesh> meshes;
        for(auto it = _root.meshes.begin(); it != _root.meshes.end(); it++) {
            DasMesh mesh;
            for(size_t i = 0; i < it->primitives.size(); i++) {
                // check if non-indexed geometry is used and throw an error if needed
                if(it->primitives[i].indices == INT32_MAX) {
                    std::cerr << "The use of non-indexed geometry is not supported" << std::endl;
                    std::exit(LIBDAS_ERROR_INVALID_DATA);
                }

                // for each atribute in array
                for(auto attr_it = it->primitives[i].attributes.begin(); attr_it != it->primitives[i].attributes.end(); attr_it++) {
                    std::string no_nr = String::RemoveNumbers(attr_it->first);

                    // check if no number string key not present
                    if(m_attribute_type_map.find(no_nr) == m_attribute_type_map.end()) {
                        std::cerr << "Invalid mesh attribute " << attr_it->first << std::endl;
                        std::exit(LIBDAS_ERROR_INVALID_DATA);
                    }

                    switch(m_attribute_type_map.find(no_nr)->second) {
                        case LIBDAS_BUFFER_TYPE_VERTEX:
                            break;
                    }
                }
            }
        }
        return meshes;
    }


    std::vector<DasNode> GLTFCompiler::_CreateNodes(const GLTFRoot &_root) {
        std::vector<DasNode> nodes;
        return nodes;
    }


    std::vector<DasScene> GLTFCompiler::_CreateScenes(const GLTFRoot &_root) {
        std::vector<DasScene> scenes;

        return scenes;
    }


    std::vector<DasSkeleton> GLTFCompiler::_CreateSkeletons(const GLTFRoot &_root) {
        std::vector<DasSkeleton> skeletons;

        return skeletons;
    }


    std::vector<DasSkeletonJoint> GLTFCompiler::_CreateSkeletonJoints(const GLTFRoot &_root) {
        std::vector<DasSkeletonJoint> joints;

        return joints;
    }


    std::vector<DasAnimation> GLTFCompiler::_CreateAnimations(const GLTFRoot &_root) {
        std::vector<DasAnimation> animations;
        return animations;
    }


    void GLTFCompiler::Compile(const GLTFRoot &_root, const DasProperties &_props, const std::string &_out_file) {
        // check if new file should be opened
        if(_out_file != "")
            NewFile(_out_file);

        _CheckAndSupplementProperties(const_cast<GLTFRoot&>(_root), const_cast<DasProperties&>(_props));
        InitialiseFile(_props);

        // write buffers to file
        std::vector<DasBuffer> buffers = _CreateBuffers(_root);
        for(auto it = buffers.begin(); it != buffers.end(); it++)
            WriteBuffer(*it);

        // write meshes to the file
        std::vector<DasMesh> meshes = _CreateMeshes(_root);
        for(auto it = meshes.begin(); it != meshes.end(); it++)
            WriteMesh(*it);

        // write scene nodes to the file
        std::vector<DasNode> nodes = _CreateNodes(_root);
        for(auto it = nodes.begin(); it != nodes.end(); it++)
            WriteNode(*it);

        // write scenes to the file
        std::vector<DasScene> scenes = _CreateScenes(_root);
        for(auto it = scenes.begin(); it != scenes.end(); it++)
            WriteScene(*it);

        // write skeletons to the file
        std::vector<DasSkeleton> skeletons = _CreateSkeletons(_root);
        for(auto it = skeletons.begin(); it != skeletons.end(); it++)
            WriteSkeleton(*it);

        // write skeleton joints to the file
        std::vector<DasSkeletonJoint> joints = _CreateSkeletonJoints(_root);
        for(auto it = joints.begin(); it != joints.end(); it++)
            WriteSkeletonJoint(*it);

        // write animations to file
        std::vector<DasAnimation> animations = _CreateAnimations(_root);
        for(auto it = animations.begin(); it != animations.end(); it++)
            WriteAnimation(*it);
    }
}
