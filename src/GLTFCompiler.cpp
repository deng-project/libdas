#define GLTF_COMPILER_CPP
#include <GLTFCompiler.h>


namespace Libdas {

    GLTFCompiler::GLTFCompiler(const std::string &_out_file) : DasWriterCore(_out_file) {}

    GLTFCompiler::GLTFCompiler(const GLTFRoot &_root, const DasProperties &_props, const std::string &_out_file) {
        Compile(_root, _props, _out_file);
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
        std::unordered_map<std::string, BufferType> attribute_type_map = {
            std::make_pair("POSITION", LIBDAS_BUFFER_TYPE_VERTEX),
            std::make_pair("NORMAL", LIBDAS_BUFFER_TYPE_VERTEX_NORMAL),
            std::make_pair("TANGENT", LIBDAS_BUFFER_TYPE_VERTEX_TANGENT),
            std::make_pair("TEXCOORD_", LIBDAS_BUFFER_TYPE_TEXTURE_MAP),
            std::make_pair("COLOR_", LIBDAS_BUFFER_TYPE_COLOR),
            std::make_pair("JOINTS_", LIBDAS_BUFFER_TYPE_JOINTS),
            std::make_pair("WEIGHTS_", LIBDAS_BUFFER_TYPE_WEIGHTS)
        };

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
                    if(attribute_type_map.find(no_nr) == attribute_type_map.end()) {
                        std::cerr << "GLTF error:  No valid attribute '" << map_it->first << "' available for current implementation" << std::endl;
                        std::exit(1);
                    }
                }
            }
        }
    }


    std::vector<DasBuffer> GLTFCompiler::_CreateBuffers(const GLTFRoot &_root) {
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

        // append images
        for(auto it = _root.images.begin(); it != _root.images.end(); it++) {
            DasBuffer buffer;

            // there are two possibilities:
            // 1. the image is defined with its uri
            // 2. the image is defined in some buffer view
            if(it->uri != "") {
                const std::vector<char> &resource = _root.resources.at(it->uri);
                buffer.data_len = resource.size();
                buffer.data_ptrs.push_back(std::make_pair(resource.data(), resource.size()));

                BufferImageTypeResolver resolver;
                resolver.FindImageBufferTypeFromResource(resource, it->uri);
                buffer.type |= resolver.GetResolvedType();
            }
            else {
                BufferImageTypeResolver resolver;
                resolver.ResolveFromBufferView(it->mime_type);
                buffer.type |= resolver.GetResolvedType();
            }

            buffers.push_back(buffer);
        }

        _FlagBuffersAccordingToMeshes(_root, buffers);

        return buffers;
    }


    std::vector<DasMesh> GLTFCompiler::_CreateMeshes(const GLTFRoot &_root) {
        std::vector<DasMesh> meshes;

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
