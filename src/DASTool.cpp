// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DASTool.cpp - CLI tool for managing DAS files and creating new DAS files from other 3D model formats 
// author: Karl-Mihkel Ott

#define DAS_TOOL_CPP
#include <DASTool.h>

///////////////////////////////
// ***** DASTool class ***** //
///////////////////////////////


// incomplete method
void DASTool::_ConvertDAS(const std::string &_input_file) {
    std::cerr << "Feature not yet implemented" << std::endl;
    LIBDAS_ASSERT(false);
    _MakeOutputFile(_input_file);
    Libdas::DasParser parser(_input_file);
    parser.Parse(true);

    // check if there are any textures to be supplemented or removed
    if(m_flags & USAGE_FLAG_REMOVE_TEXTURE) {
    }
}


void DASTool::_ConvertSTL(const std::string &_input_file) {
    _MakeOutputFile(_input_file);

    bool is_ascii = Libdas::STLFunctions::Identify(_input_file);
    if(is_ascii) {
        Libdas::AsciiSTLParser parser(_input_file);
        parser.Parse();
        Libdas::STLCompiler(parser.GetObjects(), m_props, m_out_file);
    } else {
        Libdas::BinarySTLParser parser(_input_file);
        parser.Parse();

        std::vector<Libdas::STLObject> objects = {parser.GetObject()};
        Libdas::STLCompiler(objects, m_props, m_out_file);
    }
}


void DASTool::_ConvertWavefrontObj(const std::string &_input_file) {
    _MakeOutputFile(_input_file);
    _MakeProps();

    Libdas::WavefrontObjParser parser(_input_file);
    parser.Parse();
    Libdas::WavefrontObjCompiler(parser.GetParsedData(), m_props, m_out_file, m_embedded_textures);
}


void DASTool::_ConvertGLTF(const std::string &_input_file) {
    _MakeOutputFile(_input_file);
    _MakeProps();

    Libdas::GLTFParser parser(_input_file);
    parser.Parse();
    Libdas::GLTFCompiler compiler(Libdas::Algorithm::ExtractRootPath(_input_file), parser.GetRootObject(), m_props, m_out_file);
}


void DASTool::_ConvertGLB(const std::string &_input_file) {
    std::cerr << "Feature not yet implemented" << std::endl;
    std::cerr << "Convert file: " << _input_file << std::endl;
    EXIT_ON_ERROR(-1);
}


void DASTool::_ConvertFBX(const std::string &_input_file) {
    std::cerr << "Feature not yet implemented" << std::endl;
    std::cerr << "Convert file: " << _input_file << std::endl;
    EXIT_ON_ERROR(-1);
}


void DASTool::_ListWavefrontObjRenderAttributes(const Libdas::WavefrontObjGroup &_group) {
    std::cout << "Use bevel interpolation: " << (_group.rend_attrs.use_bevel ? "true" : "false") << std::endl;
    std::cout << "Use color interpolation: " << (_group.rend_attrs.use_color_interp ? "true" : "false") << std::endl;
    std::cout << "Use diffuse interpolation: " << (_group.rend_attrs.use_diffuse_interp ? "true" : "false") << std::endl;
    std::cout << "Level of detail: " << _group.rend_attrs.lod << std::endl;

    std::string map_id_name = "", mtl_id_name = "";

    // map libraries are specified
    if(!_group.rend_attrs.maplibs.empty()) {
        std::cout << "Map libraries: ";
        for(auto it = _group.rend_attrs.maplibs.begin(); it != _group.rend_attrs.maplibs.end(); it++) {
            if(it->second == _group.rend_attrs.map_id)
                map_id_name = it->first;
            
            std::cout << "\"" << it->first << "\" ";
        }

        std::cout << std::endl;
    }

    // material libraries are specified
    if(!_group.rend_attrs.mtllibs.empty()) {
        std::cout << "MTL libraries: ";
        for(auto it = _group.rend_attrs.mtllibs.begin(); it != _group.rend_attrs.mtllibs.end(); it++) {
            if(it->second == _group.rend_attrs.mtl_id)
                mtl_id_name = it->first;

            std::cout << "\"" << it->first << "\" ";
        }

        std::cout << std::endl;
    }

    std::cout << "Shadow object: " << "\"" << _group.rend_attrs.shadow_obj << "\"" << std::endl;
    std::cout << "Raytracing object: " << "\"" << _group.rend_attrs.trace_obj << "\"" << std::endl;
}


void DASTool::_ListSTL(const std::string &_input_file) {
    bool is_ascii = Libdas::STLFunctions::Identify(_input_file);
    std::vector<Libdas::STLObject> objects;

    // parse the file
    if(is_ascii) {
        Libdas::AsciiSTLParser parser(_input_file);
        parser.Parse();
        objects = parser.GetObjects();
    } else {
        Libdas::BinarySTLParser parser(_input_file);
        parser.Parse();
        objects.push_back(parser.GetObject());
    }

    // output information about objects to stdout
    for(size_t i = 0; i < objects.size(); i++) {
        std::cout << "STL object: \"" << objects[i].name << "\"" << std::endl;
        std::cout << "Facet count: " << objects[i].facets.size() << std::endl;

        if(i != objects.size() - 1)
            std::cout << std::endl;
    }
}


void DASTool::_ListWavefrontObj(const std::string &_input_file) {
    Libdas::WavefrontObjParser parser(_input_file);
    parser.Parse();

    const Libdas::WavefrontObjData &data = parser.GetParsedData();
    const std::vector<Libdas::WavefrontObjGroup> &groups = data.groups;

    // output information about vertices
    if(data.vertices.position.size())
        std::cout << "Position vertices count: " << data.vertices.position.size() << std::endl;
    if(data.vertices.points.size())
        std::cout << "Point vertices count: " << data.vertices.points.size() << std::endl;
    if(data.vertices.normals.size())
        std::cout << "Vertex normals count: " << data.vertices.normals.size() << std::endl;
    if(data.vertices.texture.size())
        std::cout << "Texture vertices count: " << data.vertices.texture.size() << std::endl;
    
    // output information about groups to stdout
    for(size_t i = 0; i < groups.size(); i++) {
        if(i > 0) std::cout << std::endl;
        std::string name = Libdas::Algorithm::ConcatenateNameArgs(groups[i].names);
        std::cout << "Group name: " << name << std::endl;

        // verbose flag is used output render attributes
        if(m_flags & USAGE_FLAG_VERBOSE)
            _ListWavefrontObjRenderAttributes(groups[i]);

        // output indices count
        if(groups[i].indices.faces.size())
            std::cout << "Faces count: " << groups[i].indices.faces.size() << std::endl;
        if(groups[i].indices.pt.size())
            std::cout << "Points count: " << groups[i].indices.pt.size() << std::endl;
        if(groups[i].indices.lines.size())
            std::cout << "Lines count: " << groups[i].indices.lines.size() << std::endl;
    }
}


void DASTool::_ListGLTF(const std::string &_input_file) {
    std::cerr << "Feature not yet implemented" << std::endl;
    std::cerr << "File: " << _input_file << std::endl;
    EXIT_ON_ERROR(-1);
}


void DASTool::_ListGLB(const std::string &_input_file) {
    std::cerr << "Feature not yet implemented" << std::endl;
    std::cerr << "File: " << _input_file << std::endl;
    EXIT_ON_ERROR(-1);
}


void DASTool::_ListDasProperties(const Libdas::DasProperties &_props) {
    // output file properties
    if(_props.model != "")
        std::cout << "Model: " << _props.model << std::endl;
    if(_props.author != "")
        std::cout << "Author: " << _props.author << std::endl;
    if(_props.copyright != "")
        std::cout << "Copyright: " << _props.copyright << std::endl;

    const int n = 64;
    char date_and_time[n] = {};

    // convert unix timestamp into human readable string
    time_t time = static_cast<time_t>(_props.moddate);
    std::tm *dt = localtime(&time);
    strftime(date_and_time, n, "%A %Y-%m-%d %H:%M:%S (UTC)\n", dt);
    std::cout << "Modification date and time: " << date_and_time;
}


void DASTool::_ListDasBuffers(Libdas::DasParser &_parser) {
    for(uint32_t i = 0; i < _parser.GetBufferCount(); i++) {
        std::cout << std::endl << "-- Buffer nr " << i << " --" << std::endl;
        std::string types;

        if((_parser.AccessBuffer(i).type & LIBDAS_BUFFER_TYPE_VERTEX) == LIBDAS_BUFFER_TYPE_VERTEX)
            types += " vertex";
        if((_parser.AccessBuffer(i).type & LIBDAS_BUFFER_TYPE_TEXTURE_MAP) == LIBDAS_BUFFER_TYPE_TEXTURE_MAP)
            types += " texmap";
        if((_parser.AccessBuffer(i).type & LIBDAS_BUFFER_TYPE_VERTEX_NORMAL) == LIBDAS_BUFFER_TYPE_VERTEX_NORMAL)
            types += " normals";
        if((_parser.AccessBuffer(i).type & LIBDAS_BUFFER_TYPE_VERTEX_TANGENT) == LIBDAS_BUFFER_TYPE_VERTEX_TANGENT)
            types += " tangents";
        if((_parser.AccessBuffer(i).type & LIBDAS_BUFFER_TYPE_COLOR) == LIBDAS_BUFFER_TYPE_COLOR)
            types += " colordata";
        if((_parser.AccessBuffer(i).type & LIBDAS_BUFFER_TYPE_JOINTS) == LIBDAS_BUFFER_TYPE_JOINTS)
            types += " joints";
        if((_parser.AccessBuffer(i).type & LIBDAS_BUFFER_TYPE_WEIGHTS) == LIBDAS_BUFFER_TYPE_WEIGHTS)
            types += " jointweights";
        if((_parser.AccessBuffer(i).type & LIBDAS_BUFFER_TYPE_INDICES) == LIBDAS_BUFFER_TYPE_INDICES)
            types += " indices";
        if((_parser.AccessBuffer(i).type & LIBDAS_BUFFER_TYPE_TEXTURE_JPEG) == LIBDAS_BUFFER_TYPE_TEXTURE_JPEG)
            types += " jpeg";
        if((_parser.AccessBuffer(i).type & LIBDAS_BUFFER_TYPE_TEXTURE_PNG) == LIBDAS_BUFFER_TYPE_TEXTURE_PNG)
            types += " png";
        if((_parser.AccessBuffer(i).type & LIBDAS_BUFFER_TYPE_TEXTURE_BMP) == LIBDAS_BUFFER_TYPE_TEXTURE_BMP)
            types += " bmp";
        if((_parser.AccessBuffer(i).type & LIBDAS_BUFFER_TYPE_TEXTURE_PPM) == LIBDAS_BUFFER_TYPE_TEXTURE_PPM)
            types += " bmp";
        if((_parser.AccessBuffer(i).type & LIBDAS_BUFFER_TYPE_TEXTURE_RAW) == LIBDAS_BUFFER_TYPE_TEXTURE_RAW)
            types += " textureraw";

        std::cout << "Buffer types:" << types << std::endl;
        std::cout << "Data length: " << _parser.AccessBuffer(i).data_len << std::endl;
    }
}


void DASTool::_ListDasScenes(Libdas::DasParser &_parser) {
    for(size_t i = 0; i < _parser.GetSceneCount(); i++) {
        const Libdas::DasScene &scene = _parser.AccessScene(i);
        std::cout << std::endl << "-- Scene nr " << i << " --" << std::endl;

        if(scene.name != "")
            std::cout << "Scene name: " << scene.name << std::endl;

        std::cout << "Node count: " << scene.node_count << std::endl;

        // output used nodes
        std::cout << "Nodes: ";
        for(uint32_t j = 0; j < scene.node_count; j++)
            std::cout << scene.nodes[j] << " ";
        std::cout << std::endl;
    }
}


void DASTool::_ListDasNodes(Libdas::DasParser &_parser) {
    for(uint32_t i = 0; i < _parser.GetNodeCount(); i++) {
        const Libdas::DasNode &node = _parser.AccessNode(i);
        std::cout << std::endl << "-- Node nr " << i << " --" << std::endl;

        if(node.name != "") std::cout << "Name: " << node.name << std::endl;
        if(node.children_count) {
            std::cout << "Children count: " << node.children_count << std::endl;
            std::cout << "Children: ";
            for(uint32_t j = 0; j < node.children_count; j++)
                std::cout << node.children[j] << " ";
            std::cout << std::endl;
        }
        if(node.mesh != UINT32_MAX)
            std::cout << "Mesh: " << node.mesh << std::endl;
        if(node.skeleton != UINT32_MAX)
            std::cout << "Skeleton: " << node.skeleton << std::endl;

        // output transformation matrix
        std::cout << "Transformation matrix: " << std::endl;
        for(struct {Libdas::Matrix4<float>::iterator it; uint32_t i; } s = {node.transform.BeginRowMajor(), 0}; s.it != node.transform.EndRowMajor(); s.it++, s.i++) {
            if(s.i % 4 == 3) 
                std::cout << *s.it << std::endl;
            else std::cout << *s.it << " ";
        }
    }
}


void DASTool::_ListDasMeshes(Libdas::DasParser &_parser) {
    for(uint32_t i = 0; i < _parser.GetMeshCount(); i++) {
        std::cout << std::endl << "-- Mesh nr " << i << " --" << std::endl;
        const Libdas::DasMesh &mesh = _parser.AccessMesh(i);
        std::cout << "Mesh name: " << mesh.name << std::endl;
        std::cout << "Mesh primitive count: " << mesh.primitive_count << std::endl;
        std::cout << "Mesh primitives: ";
        for(uint32_t j = 0; j < mesh.primitive_count; j++)
            std::cout << mesh.primitives[j] << " ";
        std::cout << std::endl;

        // for each primitive in mesh output its data
        for(uint32_t j = 0; j < mesh.primitive_count; j++)
            _ListDasMeshPrimitive(_parser, j, mesh.primitives[j]);
    }
}


void DASTool::_ListDasMeshPrimitive(Libdas::DasParser &_parser, uint32_t _rel_id, uint32_t _id) {
    const Libdas::DasMeshPrimitive &prim = _parser.AccessMeshPrimitive(_id);
    std::cout << "---- Primitive nr " << _rel_id << " ----" << std::endl;
    std::cout << "-- Index buffer id: " << prim.index_buffer_id << std::endl;
    std::cout << "-- Index buffer offset: " << prim.index_buffer_offset << std::endl;
    std::cout << "-- Indices count: " << prim.indices_count << std::endl;
    std::cout << "-- Vertex buffer id: " << prim.vertex_buffer_id << std::endl;
    std::cout << "-- Vertex buffer offset: " << prim.vertex_buffer_offset << std::endl;

    if(prim.vertex_normal_buffer_id != UINT32_MAX) {
        std::cout << "-- Vertex normal buffer id: " << prim.vertex_normal_buffer_id << std::endl;
        if(prim.vertex_normal_buffer_offset)
            std::cout << "-- Vertex normal buffer offset: " << prim.vertex_normal_buffer_offset << std::endl;
    }
    if(prim.vertex_tangent_buffer_id != UINT32_MAX) {
        std::cout << "-- Vertex tangent buffer id: " << prim.vertex_tangent_buffer_id << std::endl;
        if(prim.vertex_tangent_buffer_offset)
            std::cout << "-- Vertex tangent buffer offset: " << prim.vertex_tangent_buffer_offset << std::endl;
    }

    // texture data
    if(prim.texture_count) {
        std::cout << "-- Texture count: " << prim.texture_count << std::endl;
        std::cout << "-- UV buffer ids: ";
        for(uint32_t i = 0; i < prim.texture_count; i++)
            std::cout << prim.uv_buffer_ids[i] << " ";
        std::cout << std::endl;

        std::cout << "-- UV buffer offsets: ";
        for(uint32_t i = 0; i < prim.texture_count; i++)
            std::cout << prim.uv_buffer_offsets[i] << " ";
        std::cout << std::endl;

        if(prim.texture_ids) {
            std::cout << "-- Associated texture ids: ";
            for(uint32_t i = 0; i < prim.texture_count; i++)
                std::cout << prim.texture_ids[i] << std::endl;
        }
    }

    // color multiplier data
    if(prim.color_mul_count) {
        std::cout << "-- Color multiplier count: " << prim.color_mul_count << std::endl;
        std::cout << "-- Color multiplier buffer ids: ";
        for(uint32_t i = 0; i < prim.color_mul_count; i++)
            std::cout << prim.color_mul_buffer_ids[i] << " ";
        std::cout << std::endl;
        
        std::cout << "-- Color multiplier buffer offsets: ";
        for(uint32_t i = 0; i < prim.color_mul_count; i++)
            std::cout << prim.color_mul_buffer_offsets[i] << " ";
        std::cout << std::endl;
    }

    // joint sets data
    if(prim.joint_set_count) {
        std::cout << "-- Joint set count: " << prim.joint_set_count << std::endl;
        std::cout << "-- Joint index buffer ids: ";
        for(uint32_t i = 0; i < prim.joint_set_count; i++)
            std::cout << prim.joint_index_buffer_ids[i] << " ";
        std::cout << std::endl;

        std::cout << "-- Joint index buffer offsets: ";
        for(uint32_t i = 0; i < prim.joint_set_count; i++)
            std::cout << prim.joint_index_buffer_offsets[i] << " ";
        std::cout << std::endl;

        std::cout << "-- Joint weight buffer ids: ";
        for(uint32_t i = 0; i < prim.joint_set_count; i++)
            std::cout << prim.joint_weight_buffer_ids[i] << " ";
        std::cout << std::endl;

        std::cout << "-- Joint weight buffer offsets: ";
        for(uint32_t i = 0; i < prim.joint_set_count; i++)
            std::cout << prim.joint_weight_buffer_offsets[i] << " ";
        std::cout << std::endl;

    }

    // morph targets
    if(prim.morph_target_count) {
        std::cout << "-- Morph target count: " << prim.morph_target_count << std::endl;
        std::cout << "-- Morph targets: ";
        for(uint32_t i = 0; i < prim.morph_target_count; i++)
            std::cout << prim.morph_targets[i] << " ";
        std::cout << std::endl;

        std::cout << "-- Morph weights: ";
        for(uint32_t i = 0; i < prim.morph_target_count; i++)
            std::cout << prim.morph_weights[i] << " ";
        std::cout << std::endl;

        for(uint32_t i = 0; i < prim.morph_target_count; i++)
            _ListDasMorphTarget(_parser, i, prim.morph_targets[i]);
    }
}


void DASTool::_ListDasMorphTarget(Libdas::DasParser &_parser, uint32_t _rel_id, uint32_t _id) {
    std::cout << "------ Morph target nr " << _rel_id << " ------" << std::endl;
    const Libdas::DasMorphTarget &morph = _parser.AccessMorphTarget(_id);
    if(morph.vertex_buffer_id != UINT32_MAX) {
        std::cout << "---- Vertex buffer id: " << morph.vertex_buffer_id << std::endl;
        if(morph.vertex_buffer_offset)
            std::cout << "---- Vertex buffer offset: " << morph.vertex_buffer_offset << std::endl;
    }

    if(morph.vertex_normal_buffer_id != UINT32_MAX) {
        std::cout << "---- Vertex normal buffer id: " << morph.vertex_normal_buffer_id << std::endl;
        if(morph.vertex_normal_buffer_offset)
            std::cout << "---- Vertex normal buffer offset: " << morph.vertex_normal_buffer_offset << std::endl;
    }

    if(morph.vertex_tangent_buffer_id != UINT32_MAX) {
        std::cout << "---- Vertex tangent buffer id: " << morph.vertex_tangent_buffer_id << std::endl;
        if(morph.vertex_tangent_buffer_offset)
            std::cout << "---- Vertex tangent buffer offset: " << morph.vertex_tangent_buffer_offset << std::endl;
    }

    if(morph.texture_count) {
        std::cout << "---- Texture count: " << morph.texture_count << std::endl;
        std::cout << "---- UV buffer ids: ";
        for(uint32_t i = 0; i < morph.texture_count; i++)
            std::cout << morph.uv_buffer_ids[i] << " ";
        std::cout << std::endl;

        std::cout << "---- UV buffer offsets: ";
        for(uint32_t i = 0; i < morph.texture_count; i++)
            std::cout << morph.uv_buffer_offsets[i] << " ";
        std::cout << std::endl;
    }

    if(morph.color_mul_count) {
        std::cout << "---- Color multiplier count: " << morph.color_mul_count << std::endl;
        std::cout << "---- Color multiplier buffer ids: ";
        for(uint32_t i = 0; i < morph.color_mul_count; i++)
            std::cout << morph.color_mul_buffer_ids[i] << " ";
        std::cout << std::endl;

        std::cout << "---- Color multiplier buffer offsets: ";
        for(uint32_t i = 0; i < morph.color_mul_count; i++)
            std::cout << morph.color_mul_buffer_offsets[i] << " ";
        std::cout << std::endl;
    }
}


void DASTool::_ListDasSkeletons(Libdas::DasParser &_parser) {
    for(uint32_t i = 0; i < _parser.GetSkeletonCount(); i++) {
        std::cout << std::endl << "-- Skeleton nr " << i << " --" << std::endl;
        const Libdas::DasSkeleton &skeleton = _parser.AccessSkeleton(i);
        if(skeleton.name != "") std::cout << "Name: " << skeleton.name << std::endl;
        std::cout << "Parent: " << skeleton.parent << std::endl;
        std::cout << "Joint count: " << skeleton.joint_count << std::endl;

        std::cout << "Joints: ";
        for(uint32_t j = 0; j < skeleton.joint_count; j++)
            std::cout << skeleton.joints[j] << " ";
        std::cout << std::endl;
    }
}


void DASTool::_ListDasSkeletonJoints(Libdas::DasParser &_parser) {
    for(uint32_t i = 0; i < _parser.GetSkeletonJointCount(); i++) {
        std::cout << std::endl << "-- Skeleton joint nr " << i << " --" << std::endl;
        const Libdas::DasSkeletonJoint &joint = _parser.AccessSkeletonJoint(i);
        // output inverse bind position matrix
        std::cout << "Inverse bind position matrix: " << std::endl;
        for(struct {Libdas::Matrix4<float>::iterator it; uint32_t j; } s = {joint.inverse_bind_pos.BeginRowMajor(), 0}; s.it != joint.inverse_bind_pos.EndRowMajor(); s.it++, s.j++) {
            if(s.j % 4 == 3) std::cout << *s.it << std::endl;
            else std::cout << *s.it << " ";
        }

        if(joint.name != "") std::cout << "Name: " << joint.name << std::endl;
        std::cout << "Children count: " << joint.children_count << std::endl;
        std::cout << "Children: ";
        for(uint32_t i = 0; i < joint.children_count; i++)
            std::cout << joint.children[i] << " ";
        std::cout << std::endl;

        std::cout << "Scale: " << joint.scale << std::endl;
        std::cout << "Translation: {" << joint.translation.x << ", " << joint.translation.y << ", " << joint.translation.z << "}" << std::endl;
    }
}


void DASTool::_ListDasAnimationChannels(Libdas::DasParser &_parser) {
    for(uint32_t i = 0; i < _parser.GetAnimationChannelCount(); i++) {
        std::cout << std::endl << "-- Animation channel nr " << i << " --" << std::endl;
        const Libdas::DasAnimationChannel &channel = _parser.AccessAnimationChannel(i);

        if(channel.node_id != UINT32_MAX)
            std::cout << "Referenced node id: " << channel.node_id << std::endl;
        else if(channel.joint_id != UINT32_MAX)
            std::cout << "Referenced joint id: " << channel.joint_id << std::endl;

        // output information about animation target 
        std::cout << "Animation target: ";
        switch(channel.target) {
            case LIBDAS_ANIMATION_TARGET_ROTATION:
                std::cout << "rotation" << std::endl;
                break;

            case LIBDAS_ANIMATION_TARGET_SCALE:
                std::cout << "scale" << std::endl;
                break;

            case LIBDAS_ANIMATION_TARGET_TRANSLATION:
                std::cout << "translation" << std::endl;
                break;

            case LIBDAS_ANIMATION_TARGET_WEIGHTS:
                std::cout << "weights" << std::endl;
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }

        // output information about interpolation type
        std::cout << "Interpolation: ";
        switch(channel.interpolation) {
            case LIBDAS_INTERPOLATION_VALUE_LINEAR:
                std::cout << "linear" << std::endl;
                break;

            case LIBDAS_INTERPOLATION_VALUE_STEP:
                std::cout << "step" << std::endl;
                break;

            case LIBDAS_INTERPOLATION_VALUE_CUBICSPLINE:
                std::cout << "cubic spline" << std::endl;
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
        
        std::cout << "Keyframe count: " << channel.keyframe_count << std::endl;
        if(channel.weight_count)
            std::cout << "Morph target weight count: " << channel.weight_count << std::endl;

        std::cout << "Keyframes: ";
        for(uint32_t j = 0; j < channel.keyframe_count; j++)
            std::cout << channel.keyframes[j] << " ";
        std::cout << std::endl;

        if(channel.interpolation == LIBDAS_INTERPOLATION_VALUE_CUBICSPLINE && channel.tangents) {
            std::cout << "Keyframe tangents: ";
            const char *tang = channel.tangents;
            for(uint32_t j = 0; j < channel.keyframe_count; j++) {
                switch(channel.target) {
                    case LIBDAS_ANIMATION_TARGET_WEIGHTS:
                        {
                            const float *in = reinterpret_cast<const float*>(tang);
                            tang += channel.weight_count * sizeof(float);
                            const float *out = reinterpret_cast<const float*>(tang);
                            tang += channel.weight_count * sizeof(float);

                            std::cout << "{{ ";
                            for(uint32_t k = 0; k < channel.weight_count; k++) {
                                if(k != channel.weight_count - 1)
                                    std::cout << in[k] << ", ";
                                else std::cout << in[k] << "}, ";
                            }
                            for(uint32_t k = 0; k < channel.weight_count; k++) {
                                if(k != channel.weight_count - 1)
                                    std::cout << out[k] << ", ";
                                else std::cout << out[k] << "}";
                            }
                            std::cout << "} ";
                        }
                        break;

                    case LIBDAS_ANIMATION_TARGET_TRANSLATION:
                        {
                            const Libdas::Vector3<float> *in = reinterpret_cast<const Libdas::Vector3<float>*>(tang);
                            tang += sizeof(Libdas::Vector3<float>);
                            const Libdas::Vector3<float> *out = reinterpret_cast<const Libdas::Vector3<float>*>(tang);
                            tang += sizeof(Libdas::Vector3<float>);
                            std::cout << "{{ " << in->first << ", " << in->second << ", " << in->third << "}, " <<
                                         "{" << out->first << ", " << out->second << ", " << out->third << "}} ";
                        }
                        break;

                    case LIBDAS_ANIMATION_TARGET_ROTATION:
                        {
                            const Libdas::Quaternion *in = reinterpret_cast<const Libdas::Quaternion*>(tang); 
                            tang += sizeof(Libdas::Quaternion);
                            const Libdas::Quaternion *out = reinterpret_cast<const Libdas::Quaternion*>(tang);
                            tang += sizeof(Libdas::Quaternion);
                            std::cout << "{{ " << in->x << ", " << in->y << ", " << in->z << ", " << in->w << "}, " <<
                                         "{" << out->x << ", " << out->y << ", " << out->z << ", " << out->w << "}} ";
                        }
                        break;

                    case LIBDAS_ANIMATION_TARGET_SCALE:
                        {
                            float in = *reinterpret_cast<const float*>(tang);
                            tang += sizeof(float);
                            float out = *reinterpret_cast<const float*>(tang);
                            tang += sizeof(float);
                            std::cout << "{ " << in << ", " << out << "} ";
                        }
                        break;
                }
            }
            std::cout << std::endl;
        }

        std::cout << "Keyframe values: ";
        const char *target = channel.target_values;
        for(uint32_t j = 0; j < channel.keyframe_count; j++) {
            switch(channel.target) {
                case LIBDAS_ANIMATION_TARGET_WEIGHTS:
                    std::cout << "{ ";
                    for(uint32_t k = 0; k < channel.weight_count; k++) {
                        float w = *reinterpret_cast<const float*>(target);
                        target += sizeof(float);

                        if(k != channel.weight_count - 1)
                            std::cout << w << ", ";
                        else std::cout << w << " }";
                    }
                    std::cout << "} ";
                    break;

                case LIBDAS_ANIMATION_TARGET_ROTATION:
                    {
                        const Libdas::Quaternion *r = reinterpret_cast<const Libdas::Quaternion*>(target);
                        target += sizeof(Libdas::Quaternion);
                        std::cout << "{ " << r->x << ", " << r->y << ", " << r->z << ", " << r->w << " } ";
                    }
                    break;

                case LIBDAS_ANIMATION_TARGET_TRANSLATION:
                    {
                        const Libdas::Vector3<float> *t = reinterpret_cast<const Libdas::Vector3<float>*>(target);
                        target += sizeof(Libdas::Vector3<float>);
                        std::cout << "{ " << t->first << ", " << t->second << ", " << t->third << " } ";
                    }
                    break;

                case LIBDAS_ANIMATION_TARGET_SCALE:
                    {
                        float s = *reinterpret_cast<const float*>(target);
                        std::cout << s << " " ;
                    }
                    break;
            }
        }

        std::cout << std::endl;
    }
}


void DASTool::_ListDasAnimations(Libdas::DasParser &_parser) {
    for(uint32_t i = 0; i < _parser.GetAnimationCount(); i++) {
        std::cout << std::endl << "-- Animation nr " << i << " --" << std::endl;
        const Libdas::DasAnimation &animation = _parser.AccessAnimation(i);
        if(animation.name != "") std::cout << "Name: " << animation.name << std::endl;
        
        std::cout << "Channel count: " << animation.channel_count << std::endl;
        std::cout << "Channels: ";
        for(uint32_t i = 0; i < animation.channel_count; i++)
            std::cout << animation.channels[i] << " ";
        std::cout << std::endl;
    }
}


void DASTool::_ListDas(const std::string &_input_file) {
    Libdas::DasParser parser(_input_file);
    parser.Parse();
    
    const Libdas::DasProperties &props = parser.GetProperties();
    _ListDasProperties(props);

    // output animation and model data if verbose mode is specified
    if((m_flags & USAGE_FLAG_VERBOSE) == USAGE_FLAG_VERBOSE) {
        _ListDasBuffers(parser);
        _ListDasMeshes(parser);
        _ListDasSkeletons(parser);
        _ListDasSkeletonJoints(parser);
        _ListDasAnimationChannels(parser);
        _ListDasAnimations(parser);
    }

    _ListDasScenes(parser);
    _ListDasNodes(parser);
}


void DASTool::_FetchArg(FlagType _type, const std::string &_arg) {
    switch(_type) {
        case USAGE_FLAG_AUTHOR:
            m_author = _arg;
            break;

        case USAGE_FLAG_COPYRIGHT:
            m_copyright = _arg;
            break;

        case USAGE_FLAG_COPYRIGHT_FILE:
            m_copyright = Libdas::String::ReadFileToString(_arg);
            break;

        case USAGE_FLAG_EMBED_TEXTURE:
            m_embedded_textures.push_back(_arg);
            break;

        case USAGE_FLAG_OUT_FILE:
            m_out_file = _arg;
            break;

        default:
            break;
    }
}


void DASTool::_MakeOutputFile(const std::string &_input_file) {
    // check if there exists any directories or files with given file name
    if(std::filesystem::exists(m_out_file) && std::filesystem::is_directory(m_out_file)) {
        std::string f = Libdas::Algorithm::ExtractFileName(_input_file);
        f = Libdas::Algorithm::ReplaceFileExtension(f, "das");

        if(m_out_file.back() == '/')
            m_out_file += f;
        else m_out_file += '/' + f;
    }
    
    // check if the output file name is correctly specified
    if(m_out_file == "") {
        m_out_file = Libdas::Algorithm::ReplaceFileExtension(_input_file, "das");
        m_out_file = Libdas::Algorithm::ExtractFileName(m_out_file);
    }
}


void DASTool::_MakeProps() {
    m_props.author = m_author;
    m_props.copyright = m_copyright;
    m_props.model = m_model;
}


void DASTool::_ParseFlags(const std::vector<std::string> &_opts) {
    FlagType info_flag = 0;
    bool skip_it = false;
    for(size_t i = 0; i < _opts.size(); i++) {
        if(skip_it) {
            _FetchArg(info_flag, _opts[i]); 
            skip_it = false;
            info_flag = 0;
            continue;
        }

        // check the string values
        if(_opts[i] == "-c" || _opts[i] == "--compressed")
            m_flags |= USAGE_FLAG_COMPRESSED;
        else if(_opts[i] == "--author") {
            m_flags |= USAGE_FLAG_AUTHOR;
            info_flag = USAGE_FLAG_AUTHOR; 
            skip_it = true;
        } 
        else if(_opts[i] == "--copyright") {
            m_flags |= USAGE_FLAG_COPYRIGHT;
            info_flag = USAGE_FLAG_COPYRIGHT; 
            skip_it = true;
        }
        else if(_opts[i] == "-cf" || _opts[i] == "--copyright-file") {
            m_flags |= USAGE_FLAG_COPYRIGHT_FILE;
            info_flag = USAGE_FLAG_COPYRIGHT_FILE; 
            skip_it = true;
        }
        else if(_opts[i] == "-et" || _opts[i] == "--embed-texture") {
            m_flags |= USAGE_FLAG_EMBED_TEXTURE;
            info_flag = USAGE_FLAG_EMBED_TEXTURE; 
            skip_it = true;
        }
        else if(_opts[i] == "-rt" || _opts[i] == "--remove-texture") {
            m_flags |= USAGE_FLAG_REMOVE_TEXTURE;
            info_flag = USAGE_FLAG_REMOVE_TEXTURE;
            skip_it = true;
        }
        else if(_opts[i] == "--model") {
            m_flags |= USAGE_FLAG_MODEL;
            info_flag = USAGE_FLAG_MODEL; 
            skip_it = true;
        }
        else if(_opts[i] == "-o" || _opts[i] == "--output") {
            m_flags |= USAGE_FLAG_OUT_FILE;
            info_flag = USAGE_FLAG_OUT_FILE; 
            skip_it = true;
        }
        else if(_opts[i] == "-v" || _opts[i] == "--verbose")
            m_flags |= USAGE_FLAG_VERBOSE;
        else {
            std::cerr << "Invalid flag '" << _opts[i] << "'" << std::endl;
            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_KEYWORD);
        }
    }
}


void DASTool::_ExcludeFlags(bool _is_convert) {
    // invalid flag is:
    // * -v / --verbose
    if(_is_convert) {
        if((m_flags & USAGE_FLAG_VERBOSE) == USAGE_FLAG_VERBOSE)
            std::cerr << "Invalid use of verbose flag in convertion mode" << std::endl;
    }

    // invalid flags are:
    // * -c / --compressed
    // * --author
    // * --copyright
    // * -cf / --copyright-file
    // * -et / --embed-texture
    // * --model
    // * -o / --output
    else {
        if((m_flags & USAGE_FLAG_COMPRESSED) == USAGE_FLAG_COMPRESSED) {
            std::cerr << "Invalid use of compression flag in listing mode" << std::endl;
            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_KEYWORD);
        }
        else if((m_flags & USAGE_FLAG_AUTHOR) == USAGE_FLAG_AUTHOR) {
            std::cerr << "Invalid use of author flag in listing mode" << std::endl;
            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_KEYWORD);
        }
        else if((m_flags & USAGE_FLAG_COPYRIGHT) == USAGE_FLAG_COPYRIGHT) {
            std::cerr << "Invalid use of copyright specifier flag in listing mode" << std::endl;
            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_KEYWORD);
        }
        else if((m_flags & USAGE_FLAG_COPYRIGHT_FILE) == USAGE_FLAG_COPYRIGHT_FILE) {
            std::cerr << "Invalid use of no curves flag in listing mode" << std::endl;
            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_KEYWORD);
        }
        else if((m_flags & USAGE_FLAG_EMBED_TEXTURE) == USAGE_FLAG_EMBED_TEXTURE) {
            std::cerr << "Invalid use of embed texture flag in listing mode" << std::endl;
            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_KEYWORD);
        }
        else if((m_flags & USAGE_FLAG_REMOVE_TEXTURE) == USAGE_FLAG_REMOVE_TEXTURE) {
            std::cerr << "Invalid use of remove texture flag in listing mode" << std::endl;
            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_KEYWORD);
        }
        else if((m_flags & USAGE_FLAG_MODEL) == USAGE_FLAG_MODEL) {
            std::cerr << "Invalid use of model specifier flag in listing mode" << std::endl;
            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_KEYWORD);
        }
        else if((m_flags & USAGE_FLAG_OUT_FILE) == USAGE_FLAG_OUT_FILE) {
            std::cerr << "Invalid use of output file specifier flag in listing mode" << std::endl;
            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_KEYWORD);
        }
    }
}


void DASTool::Convert(const std::string &_input_file, const std::vector<std::string> &_opts) {
    _ParseFlags(_opts);
    _ExcludeFlags(true);

    if(_input_file == "-h" || _input_file == "--help") {
        std::cout << GetHelpText() << std::endl;
        std::exit(0);
    }

    std::string ext = Libdas::Algorithm::ExtractFileExtension(_input_file);
    if(ext == "stl")
        _ConvertSTL(_input_file);
    else if(ext == "obj")
        _ConvertWavefrontObj(_input_file);
    else if(ext == "gltf")
        _ConvertGLTF(_input_file);
    else if(ext == "glb")
        _ConvertGLB(_input_file);
    else if(ext == "fbx")
        _ConvertFBX(_input_file);

    return;
}


void DASTool::List(const std::string &_input_file, const std::vector<std::string> &_opts) {
    _ParseFlags(_opts);
    _ExcludeFlags(false);

    if(_input_file == "-h" || _input_file == "--help") {
        std::cout << GetHelpText();
        EXIT_ON_ERROR(0);
    }

    const std::string ext = Libdas::Algorithm::ExtractFileExtension(_input_file);
    if(ext == "stl") {
        _ListSTL(_input_file);
    } else if(ext == "obj") {
        _ListWavefrontObj(_input_file);
    } else if(ext == "das") {
        _ListDas(_input_file);
    } else {
        std::cerr << "Invalid file '" << _input_file << "'" << std::endl;
        EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_FILE);
    }
}


// main method
int main(int argc, char *argv[]) {
    DASTool tool;
    if(argc < 3 || (std::string(argv[1]) != "convert" && std::string(argv[1]) != "list")) {
        std::cout << tool.GetHelpText();
        return 0;
    }

    std::vector<std::string> opts;
    if(argc > 3) {
        opts.reserve(argc - 3);
        for(int i = 3; i < argc; i++)
            opts.push_back(argv[i]);
    }

    if(std::string(argv[1]) == "convert")
        tool.Convert(argv[2], opts);
    else if(std::string(argv[1]) == "list")
        tool.List(argv[2], opts);

    return 0;
}
