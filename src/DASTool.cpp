/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: DASTool.h - CLI tool for managing DAS files and creating new DAS files from other 3D model formats 
/// author: Karl-Mihkel Ott

#define DAS_TOOL_CPP
#include <DASTool.h>

///////////////////////////////
// ***** DASTool class ***** //
///////////////////////////////


// incomplete method
void DASTool::_ConvertSTL(const std::string &_input_file) {
    std::string &out_file = m_out_file;

    // check if the output file name is correctly specified
    if(out_file == "") {
        out_file = Libdas::String::ReplaceFileExtension(_input_file, "das");
        out_file = Libdas::String::ExtractFileName(out_file);
    }

    bool is_ascii = Libdas::STLFunctions::Identify(_input_file);
    if(is_ascii) {
        Libdas::AsciiSTLParser parser(_input_file);
        parser.Parse();
        Libdas::STLCompiler(parser.GetObjects(), m_props, out_file);
    } else {
        Libdas::BinarySTLParser parser(_input_file);
        parser.Parse();

        std::vector<Libdas::STLObject> objects = {parser.GetObject()};
        Libdas::STLCompiler(objects, m_props, out_file);
    }
}


void DASTool::_ConvertWavefrontObj(const std::string &_input_file) {
    std::string &out_file = m_out_file; 

    if(out_file == "") {
        out_file = Libdas::String::ReplaceFileExtension(_input_file, "das");
        out_file = Libdas::String::ExtractFileName(out_file);
    }

    Libdas::WavefrontObjParser parser(_input_file);
    parser.Parse();

    Libdas::WavefrontObjCompiler(parser.GetParsedGroups(), m_props, out_file);
}


void DASTool::_ConvertGLTF(const std::string &_input_file) {
    std::cerr << "Feature not yet implemented" << std::endl;
    LIBDAS_ASSERT(false);
}


void DASTool::_ConvertGLB(const std::string &_input_file) {
    std::cerr << "Feature not yet implemented" << std::endl;
    LIBDAS_ASSERT(false);
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

    const std::vector<Libdas::WavefrontObjGroup> &groups = parser.GetParsedGroups();
    
    // output information about groups to stdout
    for(size_t i = 0; i < groups.size(); i++) {
        if(i > 0) std::cout << std::endl;
        std::string name = Libdas::String::ConcatenateNameArgs(groups[i].names);
        std::cout << "Group name: " << name << std::endl;

        // verbose flag is used output render attributes
        if(m_flags & OUTPUT_FLAG_VERBOSE)
            _ListWavefrontObjRenderAttributes(groups[i]);

        // output vertices count
        if(groups[i].vertices.position.size())
            std::cout << "Position vertices count: " << groups[i].vertices.position.size() << std::endl;
        if(groups[i].vertices.points.size())
            std::cout << "Point vertices count: " << groups[i].vertices.points.size() << std::endl;
        if(groups[i].vertices.normals.size())
            std::cout << "Vertex normals count: " << groups[i].vertices.normals.size() << std::endl;
        if(groups[i].vertices.texture.size())
            std::cout << "Texture vertices count: " << groups[i].vertices.texture.size() << std::endl;

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
    LIBDAS_ASSERT(false);
}


void DASTool::_ListGLB(const std::string &_input_file) {
    std::cerr << "Feature not yet implemented" << std::endl;
    LIBDAS_ASSERT(false);
}


void DASTool::_ListDasProperties(Libdas::DasProperties &_props) {
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
    std::cout << "Compression: " << (_props.compression ? "true" : "false") << std::endl;
}


void DASTool::_ListDasScenes(Libdas::DasParser &_parser) {
    for(size_t i = 0; i < _parser.GetScenes().size(); i++) {
        Libdas::DasScene &scene = _parser.GetScenes()[i];
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
        Libdas::DasNode &node = _parser.AccessNode(i);
        std::cout << std::endl << "-- Node nr " << i << " --" << std::endl;

        if(node.name != "") std::cout << "Name: " << node.name << std::endl;
        if(node.children_count) {
            std::cout << "Children count: " << node.children_count << std::endl;
            std::cout << "Children: ";
            for(uint32_t j = 0; j < node.children_count; j++)
                std::cout << node.children[j] << " ";
            std::cout << std::endl;
        }
        if(node.mesh_count) {
            std::cout << "Mesh count: " << node.mesh_count << std::endl;
            std::cout << "Meshes: ";
            for(uint32_t j = 0; j < node.mesh_count; j++)
                std::cout << node.meshes[j] << " ";
            std::cout << std::endl;
        }
        if(node.animation_count) {
            std::cout << "Animation count: " << node.animation_count << std::endl;
            std::cout << "Animations: ";
            for(uint32_t j = 0; j < node.animation_count; j++)
                std::cout << node.animations[j] << " ";
            std::cout << std::endl;
        }
        if(node.skeleton_count) {
            std::cout << "Skeleton count: " << node.skeleton_count << std::endl;
            std::cout << "Skeletons: ";
            for(uint32_t j = 0; j < node.skeleton_count; j++)
                std::cout << node.skeletons[j] << " ";
            std::cout << std::endl;
        }

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
        Libdas::DasMesh &mesh = _parser.AccessMesh(i);
        std::cout << "Mesh name: " << mesh.name << std::endl;
        std::cout << "Used index buffer id: " << mesh.index_buffer_id << std::endl;
        std::cout << "Used index buffer offset: " << mesh.index_buffer_offset << std::endl;
        std::cout << "Total indices used: " << mesh.indices_count << std::endl; 
        std::cout << "Used vertex buffer id: " << mesh.vertex_buffer_id << std::endl;
        std::cout << "Used vertex buffer offset: " << mesh.vertex_buffer_offset << std::endl;

        if(mesh.texture_id != UINT32_MAX)
            std::cout << "Used texture id: " << mesh.texture_id << std::endl;

        if(mesh.texture_map_buffer_id != UINT32_MAX)
            std::cout << "Used texture map buffer id: " << mesh.texture_map_buffer_id << std::endl;

        if(mesh.texture_map_buffer_offset != 0)
            std::cout << "Used texture map buffer offset: " << mesh.texture_map_buffer_offset << std::endl;

        if(mesh.vertex_normal_buffer_id != UINT32_MAX)
            std::cout << "Used vertex normal buffer id: " << mesh.vertex_normal_buffer_id << std::endl;

        if(mesh.vertex_normal_buffer_offset != 0)
            std::cout << "Used vertex normal buffer offset: " << mesh.vertex_normal_buffer_offset << std::endl;
    }
}


void DASTool::_ListDasSkeletons(Libdas::DasParser &_parser) {
    for(uint32_t i = 0; i < _parser.GetSkeletonCount(); i++) {
        std::cout << std::endl << "-- Skeleton nr " << i << " --" << std::endl;
        Libdas::DasSkeleton &skeleton = _parser.AccessSkeleton(i);
        if(skeleton.name != "") std::cout << "Name: " << skeleton.name << std::endl;
        std::cout << "Joint count: " << skeleton.joint_count << std::endl;

        std::cout << "Joints: ";
        for(uint32_t j = 0; j < skeleton.joint_count; j++)
            std::cout << skeleton.joints[j] << std::endl;
        std::cout << std::endl;
    }
}


void DASTool::_ListDasSkeletonJoints(Libdas::DasParser &_parser) {
    for(uint32_t i = 0; i < _parser.GetSkeletonJointCount(); i++) {
        std::cout << std::endl << "-- Skeleton joint nr " << i << " --" << std::endl;
        Libdas::DasSkeletonJoint &joint = _parser.AccessSkeletonJoint(i);
        // output inverse bind position matrix
        std::cout << "Inverse bind position matrix: " << std::endl;
        for(struct {Libdas::Matrix4<float>::iterator it; uint32_t j; } s = {joint.inverse_bind_pos.BeginRowMajor(), 0}; s.it != joint.inverse_bind_pos.EndRowMajor(); s.it++, s.j++) {
            if(s.j % 4 == 3) std::cout << *s.it << std::endl;
            else std::cout << *s.it << " ";
        }

        if(joint.name != "") std::cout << "Name: " << joint.name << std::endl;
        std::cout << "Parent: " << joint.parent << std::endl;
        std::cout << "Scale: " << joint.scale << std::endl;
        std::cout << "Translation: {" << joint.translation.first << ", " << joint.translation.second << ", " << joint.translation.third << "}" << std::endl;
    }
}


void DASTool::_ListDasAnimations(Libdas::DasParser &_parser) {
    for(uint32_t i = 0; i < _parser.GetAnimationCount(); i++) {
        std::cout << std::endl << "-- Animation nr " << i << " --" << std::endl;
        Libdas::DasAnimation &animation = _parser.AccessAnimation(i);
        if(animation.name != "") std::cout << "Name: " << animation.name << std::endl;
        std::cout << "Node id: " << animation.node_id << std::endl;
        std::cout << "Duration: " << animation.duration << "s" << std::endl;
        std::cout << "Keyframe count: " << animation.keyframe_count << std::endl;
        
        // timestamps
        std::cout << "Keyframe timestamps: ";
        for(uint32_t j = 0; j < animation.keyframe_count; j++)
            std::cout << animation.keyframe_timestamps[j] << " ";
        std::cout << std::endl;

        // interpolation types
        std::cout << "Interpolation types: ";
        for(uint32_t j = 0; j < animation.keyframe_count; j++) {
            switch(animation.interpolation_types[j]) {
                case LIBDAS_INTERPOLATION_VALUE_LINEAR:
                    std::cout << "linear ";
                    break;

                case LIBDAS_INTERPOLATION_VALUE_STEP:
                    std::cout << "step ";
                    break;
    
                case LIBDAS_INTERPOLATION_VALUE_CUBICSPLINE:
                    std::cout << "cubicspline ";
                    break;

                default:
                    break;
            }
        }
        std::cout << std::endl;

        // animation targets
        std::cout << "Animation targets: ";
        for(uint32_t j = 0; j < animation.keyframe_count; j++) {
            switch(animation.animation_targets[j]) {
                case LIBDAS_ANIMATION_TARGET_WEIGHTS:
                    std::cout << "weights ";
                    break;

                case LIBDAS_ANIMATION_TARGET_TRANSLATION:
                    std::cout << "translation ";
                    break;

                case LIBDAS_ANIMATION_TARGET_ROTATION:
                    std::cout << "rotation ";
                    break;

                case LIBDAS_ANIMATION_TARGET_SCALE:
                    std::cout << "scale ";
                    break;

                default:
                    break;
            }
        }
        std::cout << std::endl;

        std::cout << "Keyframe buffer id: " << animation.keyframe_buffer_id << std::endl;
        std::cout << "Keyframe buffer offset: " << animation.keyframe_buffer_offset << std::endl;
    }
}


void DASTool::_ListDas(const std::string &_input_file) {
    Libdas::DasParser parser(_input_file);
    parser.Parse();
    
    Libdas::DasProperties &props = parser.GetProperties();
    _ListDasProperties(props);


    // output animation and model data if verbose mode is specified
    if(m_flags & OUTPUT_FLAG_VERBOSE) {
        _ListDasMeshes(parser);
        _ListDasSkeletons(parser);
        _ListDasSkeletonJoints(parser);
        _ListDasAnimations(parser);
    }

    _ListDasScenes(parser);
    _ListDasNodes(parser);
}


void DASTool::_ParseFlags(const std::vector<std::string> &_opts) {
    for(size_t i = 0; i < _opts.size(); i++) {
        // invalid flag
        if(m_flag_map.find(_opts[i]) == m_flag_map.end()) {
            std::cerr << "Invalid output option '" << _opts[i] << "'" << std::endl;
            std::cout << m_help_text;
            std::exit(LIBDAS_ERROR_INVALID_KEYWORD);
        }

        else {
            // boolean value for specifying flag iteration skip 
            bool skip_it = false;
            switch(m_flag_map[_opts[i]]) {
                case OUTPUT_FLAG_COMPRESSED:
                    m_props.compression = true;
                    break;

                case OUTPUT_FLAG_AUTHOR:
                    if(i == _opts.size() - 1) {
                        std::cerr << "Expected author name after flag '" + _opts[i] + "'\n" << std::endl;
                        std::exit(LIBDAS_ERROR_INVALID_KEYWORD);
                    }

                    m_author = _opts[i + 1];
                    skip_it = true;
                    break;

                case OUTPUT_FLAG_COPYRIGHT:
                    if(i == _opts.size() - 1) {
                        std::cerr << "Expected copyright message after flag '" + _opts[i] + "'\n" << std::endl;
                        std::exit(LIBDAS_ERROR_INVALID_KEYWORD);
                    }

                    m_copyright = _opts[i + 1];
                    skip_it = true;
                    break;

                case OUTPUT_FLAG_MODEL:
                    if(i == _opts.size() - 1) {
                        std::cerr << "Expected model name after flag '" + _opts[i] + "'\n" << std::endl;
                        std::exit(LIBDAS_ERROR_INVALID_KEYWORD);
                    }

                    m_model = _opts[i + 1];
                    skip_it = true;
                    break;

                case OUTPUT_FLAG_OUT_FILE:
                    if(i == _opts.size() - 1) {
                        std::cerr << "Expected output file name after flag '" + _opts[i] + "'\n" << std::endl;
                        std::exit(LIBDAS_ERROR_INVALID_KEYWORD);
                    }

                    m_out_file = _opts[i + 1];
                    skip_it = true;
                    break;

                default:
                    break;
            }

            m_flags |= m_flag_map[_opts[i]];
            if(skip_it) i++;
        }
    }
}


void DASTool::_ExcludeFlags(bool _is_convert) {
    // invalid flag is:
    // * -v / --verbose
    if(_is_convert) {
        if((m_flags & OUTPUT_FLAG_VERBOSE) == OUTPUT_FLAG_VERBOSE)
            std::cerr << "Invalid use of verbose flag in convertion mode" << std::endl;
    }

    // invalid flags are:
    // * -c / --compressed
    // * --no-curves
    // * --author
    // * --copyright
    // * --model
    // * -o / --output
    else {
        if((m_flags & OUTPUT_FLAG_COMPRESSED) == OUTPUT_FLAG_COMPRESSED) {
            std::cerr << "Invalid use use of compression flag in listing mode" << std::endl;
            std::exit(LIBDAS_ERROR_INVALID_KEYWORD);
        }
        if((m_flags & OUTPUT_FLAG_NO_CURVES) == OUTPUT_FLAG_NO_CURVES) {
            std::cerr << "Invalid use use of no curves flag in listing mode" << std::endl;
            std::exit(LIBDAS_ERROR_INVALID_KEYWORD);
        }
        if((m_flags & OUTPUT_FLAG_AUTHOR) == OUTPUT_FLAG_AUTHOR) {
            std::cerr << "Invalid use use of author flag in listing mode" << std::endl;
            std::exit(LIBDAS_ERROR_INVALID_KEYWORD);
        }
        if((m_flags & OUTPUT_FLAG_COPYRIGHT) == OUTPUT_FLAG_COPYRIGHT) {
            std::cerr << "Invalid use use of copyright specifier flag in listing mode" << std::endl;
            std::exit(LIBDAS_ERROR_INVALID_KEYWORD);
        }
        if((m_flags & OUTPUT_FLAG_MODEL) == OUTPUT_FLAG_MODEL) {
            std::cerr << "Invalid use use of model specifier flag in listing mode" << std::endl;
            std::exit(LIBDAS_ERROR_INVALID_KEYWORD);
        }
        if((m_flags & OUTPUT_FLAG_OUT_FILE) == OUTPUT_FLAG_OUT_FILE) {
            std::cerr << "Invalid use use of output file specifier flag in listing mode" << std::endl;
            std::exit(LIBDAS_ERROR_INVALID_KEYWORD);
        }
    }
}


void DASTool::Convert(const std::string &_input_file, const std::vector<std::string> &_opts) {
    _ParseFlags(_opts);
    _ExcludeFlags(true);

    std::string ext = Libdas::String::ExtractFileExtension(_input_file);
    if(ext == "stl") {
        _ConvertSTL(_input_file);
    } else if(ext == "obj") {
        _ConvertWavefrontObj(_input_file);
    }

    return;
}


void DASTool::List(const std::string &_input_file, const std::vector<std::string> &_opts) {
    _ParseFlags(_opts);
    _ExcludeFlags(false);

    const std::string ext = Libdas::String::ExtractFileExtension(_input_file);
    if(ext == "stl") {
        _ListSTL(_input_file);
    } else if(ext == "obj") {
        _ListWavefrontObj(_input_file);
    } else if(ext == "das") {
        _ListDas(_input_file);
    } else {
        std::cerr << "Invalid file '" << _input_file << "'" << std::endl;
        std::exit(LIBDAS_ERROR_INVALID_FILE);
    }
}


const std::string &DASTool::GetHelpText() {
    return m_help_text;
}


// main method
int main(int argc, char *argv[]) {
    DASTool tool;
    if(argc < 3) {
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
