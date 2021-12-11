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
    std::cerr << "Feature not yet implemented" << std::endl; 
    LIBDAS_ASSERT(false); // necessary measures rn

    bool is_ascii = Libdas::STLFunctions::Identify(_input_file);
    if(is_ascii) {
        Libdas::AsciiSTLParser parser(_input_file);
        parser.Parse();
    } else {
        Libdas::BinarySTLParser parser(_input_file);
        parser.Parse();
    }
}


void DASTool::_ConvertWavefrontOBJ(const std::string &_input_file) {
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


void DASTool::_ListWavefrontOBJ(const std::string &_input_file) {
    Libdas::WavefrontObjParser parser(_input_file);
    parser.Parse();

    const std::vector<Libdas::WavefrontObjGroup> &groups = parser.GetParsedGroups();
    
    // output information about groups to stdout
    for(size_t i = 0; i < groups.size(); i++) {
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


// needs a tree view
void DASTool::_ListDasScenes(const std::vector<Libdas::DasScene> &_scenes) {
    if(_scenes.size())
        std::cout << std::endl;

    for(size_t i = 0; i < _scenes.size(); i++) {
        std::cout << std::endl;
        std::cout << "Scene name: " << _scenes[i].name << std::endl;
        std::cout << "Index: " << i << std::endl;

        for(const Libdas::DasSceneNode &node : _scenes[i].nodes) {
            std::cout << "-- Node name: " << node.name << std::endl;
            if(node.children_count) {
                std::cout << "-- Children: ";
                for(uint32_t j = 0; j < node.children_count; j++)
                    std::cout << node.children[j] << " ";
            }

            if(node.model_count) {
                std::cout << "-- Used static models: ";
                for(uint32_t j = 0; j < node.model_count; j++)
                    std::cout << node.models[j] << " ";
            }

            if(node.animation_count) {
                std::cout << "-- Used animations: ";
                for(uint32_t j = 0; j < node.animation_count; j++)
                    std::cout << node.animations[j] << " ";
            }
        }
    }
}


void DASTool::_ListDasModels(Libdas::DasParser &_parser) {
    if(_parser.GetModelCount())
        std::cout << std::endl;

    for(uint32_t i = 0; i < _parser.GetModelCount(); i++) {
        Libdas::DasModel &model = _parser.AccessModel(i);
        std::cout << "Model name: " << model.name << std::endl;
        std::cout << "Used index buffer id: " << model.index_buffer_id << std::endl;
        std::cout << "Used index buffer offset: " << model.index_buffer_offset << std::endl;
        std::cout << "Total indices used: " << model.indices_count << std::endl; 
        std::cout << "Used vertex buffer id: " << model.vertex_buffer_id << std::endl;
        std::cout << "Used vertex buffer offset: " << model.vertex_buffer_offset << std::endl;

        if(model.texture_id != UINT32_MAX)
            std::cout << "Used texture id: " << model.texture_id << std::endl;

        if(model.texture_map_buffer_id != UINT32_MAX)
            std::cout << "Used texture map buffer id: " << model.texture_map_buffer_id << std::endl;

        if(model.texture_map_buffer_offset != 0)
            std::cout << "Used texture map buffer offset: " << model.texture_map_buffer_offset << std::endl;

        if(model.vertex_normal_buffer_id != UINT32_MAX)
            std::cout << "Used vertex normal buffer id: " << model.vertex_normal_buffer_id << std::endl;

        if(model.vertex_normal_buffer_offset != 0)
            std::cout << "Used vertex normal buffer offset: " << model.vertex_normal_buffer_offset << std::endl;
    }
}


void DASTool::_ListDasAnimations(Libdas::DasParser &_parser) {
    if(_parser.GetAnimationCount())
        std::cout << std::endl;

    std::string interp = "linear";

    for(uint32_t i = 0; i < _parser.GetAnimationCount(); i++) {
        Libdas::DasAnimation &animation = _parser.AccessAnimation(i);

        // check the interpolation method value
        switch(animation.interpolation) {
            case LIBDAS_INTERPOLATION_VALUE_STEP:
                interp = "step";
                break;

            case LIBDAS_INTERPOLATION_VALUE_CUBICSPLINE:
                interp = "cubic spline";
                break;

            default:
                break;
        }

        std::cout << "Animation name: " << animation.name << std::endl;
        std::cout << "Affiliated model: " << animation.model << std::endl;
        std::cout << "Animation length: " << animation.length / 60 << "m" << animation.length % 60 << "s" << std::endl;
        std::cout << "Interpolation: " << interp << std::endl;

        for(const Libdas::DasKeyframe &keyframe : animation.keyframes) {
            std::cout << "Keyframe" << std::endl;
            std::cout << "-- Timestamp: " << keyframe.timestamp << std::endl;
            std::cout << "-- Used vertex buffer id: " << keyframe.vertex_buffer_id << std::endl;
            std::cout << "-- Used vertex buffer offset: " << keyframe.vertex_buffer_offset << std::endl;

            if(keyframe.texture_map_buffer_id != UINT32_MAX)
                std::cout << "-- Used texture map buffer id: " << keyframe.texture_map_buffer_id << std::endl;

            if(keyframe.texture_map_buffer_offset)
                std::cout << "-- Used texture map buffer offset: " << keyframe.texture_map_buffer_offset << std::endl;

            if(keyframe.vertex_normal_buffer_id != UINT32_MAX)
                std::cout << "-- Used vertex normal buffer id: " << keyframe.vertex_normal_buffer_id << std::endl;

            if(keyframe.vertex_normal_buffer_offset)
                std::cout << "-- Used vertex normal buffer offset: " << keyframe.vertex_normal_buffer_offset << std::endl;
        }
    }
}


void DASTool::_ListDas(const std::string &_input_file) {
    Libdas::DasParser parser(_input_file);
    parser.Parse();
    
    Libdas::DasProperties &props = parser.GetProperties();
    std::vector<Libdas::DasScene> &scenes = parser.GetScenes();

    // output file properties
    if(props.model != "")
        std::cout << "Model: " << props.model << std::endl;
    if(props.author != "")
        std::cout << "Author: " << props.author << std::endl;
    if(props.copyright != "")
        std::cout << "Copyright: " << props.copyright << std::endl;

    const int n = 64; // at least
    char date_and_time[n] = {};

    // convert unix timestamp into human readable string
    time_t time = static_cast<time_t>(props.moddate);
    std::tm *dt = localtime(&time);
    strftime(date_and_time, n, "%A %Y-%m-%d %H:%M:%S (UTC)\n", dt);
    std::cout << "Modification date and time: " << date_and_time;
    std::cout << "Compression: " << (props.compression ? "true" : "false") << std::endl;

    // output animation and model data if verbose mode is specified
    if(m_flags & OUTPUT_FLAG_VERBOSE) {
        _ListDasModels(parser);
        _ListDasAnimations(parser);
    }

    _ListDasScenes(parser.GetScenes());
}


void DASTool::_ParseFlags(const std::vector<std::string> &_opts) {
    for(size_t i = 0; i < _opts.size(); i++) {
        // invalid flag
        if(m_flag_map.find(_opts[i]) == m_flag_map.end()) {
            std::cerr << "Invalid output option '" << _opts[i] << "'" << std::endl;
            std::cout << m_help_text;
            std::exit(1);
        }

        else {
            // boolean value for specifying flag iteration skip 
            bool skip_it = false;
            switch(m_flag_map[_opts[i]]) {
                case OUTPUT_FLAG_COMPRESSED:
                    m_props.compression = true;
                    break;

                case OUTPUT_FLAG_AUTHOR:
                    if(i == _opts.size() - 1)
                        throw std::runtime_error("Expected author name after flag '" + _opts[i] + "'" + "\n");
                    m_author = _opts[i + 1];
                    skip_it = true;
                    break;

                case OUTPUT_FLAG_COPYRIGHT:
                    if(i == _opts.size() - 1)
                        throw std::runtime_error("Expected copyright message after flag '" + _opts[i] + "'" + "\n");
                    m_copyright = _opts[i + 1];
                    skip_it = true;
                    break;

                case OUTPUT_FLAG_MODEL:
                    if(i == _opts.size() - 1)
                        throw std::runtime_error("Expected model name after flag '" + _opts[i] + "'" + "\n");
                    m_model = _opts[i + 1];
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


void DASTool::Convert(const std::string &_input_file, const std::vector<std::string> &_opts) {
    _ParseFlags(_opts);

    std::string ext = Libdas::String::ExtractFileExtension(_input_file);
    if(ext == "stl") {
        _ConvertSTL(_input_file);
    } else if(ext == "obj") {
        _ConvertWavefrontOBJ(_input_file);
    }

    return;
}


void DASTool::List(const std::string &_input_file, const std::vector<std::string> &_opts) {
    _ParseFlags(_opts);
    const std::string ext = Libdas::String::ExtractFileExtension(_input_file);
    if(ext == "stl") {
        _ListSTL(_input_file);
    } else if(ext == "obj") {
        _ListWavefrontOBJ(_input_file);
    } else if(ext == "das") {
        _ListDas(_input_file);
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
