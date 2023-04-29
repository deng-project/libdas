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
    if (m_flags & USAGE_FLAG_LOD) {
        Libdas::DasParser parser(_input_file);
        parser.Parse();

        Libdas::DasModel& model = parser.GetModel();

        size_t buffer_size = 0;
        std::vector<std::vector<uint32_t>> indices;
        indices.resize(model.mesh_primitives.size());
        std::vector<std::vector<TRS::Vector3<float>>> vertices;
        vertices.resize(model.mesh_primitives.size());

        for (size_t i = 0; i < model.mesh_primitives.size(); i++) {
            Libdas::DasMeshPrimitive& prim = model.mesh_primitives[i];
            if (prim.index_buffer_id == UINT32_MAX) {
                std::cout << "Generating LODs for unindexed meshes is not supported :(" << std::endl;
                std::exit(0);
            }

            Libdas::LodGenerator gen(
                reinterpret_cast<uint32_t*>(model.buffers[prim.index_buffer_id].data_ptrs.back().first + prim.index_buffer_offset),
                reinterpret_cast<TRS::Vector3<float>*>(model.buffers[prim.vertex_buffer_id].data_ptrs.back().first + prim.vertex_buffer_offset),
                prim.draw_count);

            gen.Simplify(static_cast<float>(m_lod) / 100.f);
            indices[i] = gen.GetLodIndices();
            vertices[i] = gen.GetLodVertices();

            buffer_size += indices[i].size() * sizeof(uint32_t);
            buffer_size += vertices[i].size() * sizeof(TRS::Vector3<float>);
        }

        // create a new DasModel instance to write as output
        Libdas::DasModel lod_model;
        lod_model.props = std::move(model.props);
        lod_model.buffers.emplace_back();
        lod_model.buffers.back().data_len = static_cast<uint32_t>(buffer_size);
        lod_model.buffers.back().data_ptrs.push_back(
            std::make_pair(new char[buffer_size], buffer_size)
        );
        lod_model.buffers.back().type = LIBDAS_BUFFER_TYPE_INDICES | LIBDAS_BUFFER_TYPE_VERTEX;

        lod_model.scenes = std::move(model.scenes);
        lod_model.nodes = std::move(model.nodes);
        lod_model.meshes = std::move(model.meshes);
        lod_model.mesh_primitives.resize(model.mesh_primitives.size());
    
        size_t offset = 0;
        for (size_t i = 0; i < lod_model.mesh_primitives.size(); i++) {
            Libdas::DasMeshPrimitive& prim = lod_model.mesh_primitives[i];
            prim.index_buffer_id = 0;
            prim.index_buffer_offset = static_cast<uint32_t>(offset);
            prim.draw_count = static_cast<uint32_t>(indices[i].size());

            char* buf = lod_model.buffers.back().data_ptrs.back().first + offset;
            std::memcpy(buf, indices[i].data(), indices[i].size() * sizeof(uint32_t));
            offset += indices[i].size() * sizeof(uint32_t);
            buf += indices[i].size() * sizeof(uint32_t);

            prim.vertex_buffer_id = 0;
            prim.vertex_buffer_offset = static_cast<uint32_t>(offset);

            std::memcpy(buf, vertices[i].data(), vertices[i].size() * sizeof(TRS::Vector3<float>));
            offset += vertices[i].size() * sizeof(TRS::Vector3<float>);
        }

        size_t pos = _input_file.find(".das");
        Libdas::DasWriterCore writer(_input_file.substr(0, pos) + "_" + std::to_string(m_lod) + _input_file.substr(pos));
        writer.InitialiseFile(lod_model.props);
        writer.WriteBuffer(lod_model.buffers[0]);
        
        for (Libdas::DasScene& scene : lod_model.scenes)
            writer.WriteScene(scene);

        for (Libdas::DasNode& node : lod_model.nodes)
            writer.WriteNode(node);

        for (Libdas::DasMesh& mesh : lod_model.meshes)
            writer.WriteMesh(mesh);
        
        for (Libdas::DasMeshPrimitive& prim : lod_model.mesh_primitives)
            writer.WriteMeshPrimitive(prim);
    }
}


void DASTool::_ConvertSTL(const std::string &_input_file) {
    _MakeOutputFile(_input_file);

    bool is_ascii = Libdas::STLFunctions::Identify(_input_file);
    if(is_ascii) {
        Libdas::AsciiSTLParser parser(_input_file);
        parser.Parse();
        Libdas::STLCompiler cmp(parser.GetObjects(), m_props, m_out_file);
    } else {
        Libdas::BinarySTLParser parser(_input_file);
        parser.Parse();

        std::vector<Libdas::STLObject> objects = {parser.GetObject()};
        Libdas::STLCompiler cmp(objects, m_props, m_out_file);
    }
}


void DASTool::_ConvertWavefrontObj(const std::string &_input_file) {
    _MakeOutputFile(_input_file);
    _MakeProps();

    Libdas::WavefrontObjParser parser(_input_file);
    parser.Parse();
    Libdas::WavefrontObjCompiler cmp(parser.GetParsedData(), m_props, m_out_file, m_embedded_textures);
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
        std::cout << "TRS::Point vertices count: " << data.vertices.points.size() << std::endl;
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
            std::cout << "TRS::Points count: " << groups[i].indices.pt.size() << std::endl;
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
    std::cout << "Default scene: " << _props.default_scene << std::endl;

    const int n = 128;
    char date_and_time[n] = {};

    // convert unix timestamp into human readable string
    time_t time = static_cast<time_t>(_props.moddate);
    std::tm *dt = localtime(&time);
    strftime(date_and_time, n, "%A %Y-%m-%d %H:%M:%S (UTC)\n", dt);
    std::cout << "Modification date and time: " << date_and_time;
}


void DASTool::_ListDasBuffers(Libdas::DasParser &_parser) {
    auto& buffers = _parser.GetModel().buffers;
    for (auto it = buffers.begin(); it != buffers.end(); it++) {
        std::cout << std::endl << "-- Buffer nr " << (it - buffers.begin()) << " --" << std::endl;
        std::string types;

        if((it->type & LIBDAS_BUFFER_TYPE_VERTEX) == LIBDAS_BUFFER_TYPE_VERTEX)
            types += " vertex";
        if((it->type & LIBDAS_BUFFER_TYPE_TEXTURE_MAP) == LIBDAS_BUFFER_TYPE_TEXTURE_MAP)
            types += " texmap";
        if((it->type & LIBDAS_BUFFER_TYPE_VERTEX_NORMAL) == LIBDAS_BUFFER_TYPE_VERTEX_NORMAL)
            types += " normals";
        if((it->type & LIBDAS_BUFFER_TYPE_VERTEX_TANGENT) == LIBDAS_BUFFER_TYPE_VERTEX_TANGENT)
            types += " tangents";
        if((it->type & LIBDAS_BUFFER_TYPE_COLOR) == LIBDAS_BUFFER_TYPE_COLOR)
            types += " colordata";
        if((it->type & LIBDAS_BUFFER_TYPE_JOINTS) == LIBDAS_BUFFER_TYPE_JOINTS)
            types += " joints";
        if((it->type & LIBDAS_BUFFER_TYPE_WEIGHTS) == LIBDAS_BUFFER_TYPE_WEIGHTS)
            types += " jointweights";
        if((it->type & LIBDAS_BUFFER_TYPE_INDICES) == LIBDAS_BUFFER_TYPE_INDICES)
            types += " indices";
        if((it->type & LIBDAS_BUFFER_TYPE_TEXTURE_JPEG) == LIBDAS_BUFFER_TYPE_TEXTURE_JPEG)
            types += " jpeg";
        if((it->type & LIBDAS_BUFFER_TYPE_TEXTURE_PNG) == LIBDAS_BUFFER_TYPE_TEXTURE_PNG)
            types += " png";
        if((it->type & LIBDAS_BUFFER_TYPE_TEXTURE_BMP) == LIBDAS_BUFFER_TYPE_TEXTURE_BMP)
            types += " bmp";
        if((it->type & LIBDAS_BUFFER_TYPE_TEXTURE_PPM) == LIBDAS_BUFFER_TYPE_TEXTURE_PPM)
            types += " bmp";
        if((it->type & LIBDAS_BUFFER_TYPE_TEXTURE_RAW) == LIBDAS_BUFFER_TYPE_TEXTURE_RAW)
            types += " textureraw";

        std::cout << "Buffer types:" << types << std::endl;
        std::cout << "Data length: " << it->data_len << std::endl;
    }
}


void DASTool::_ListDasScenes(Libdas::DasParser &_parser) {
    auto& scenes = _parser.GetModel().scenes;
    for (auto it = scenes.begin(); it != scenes.end(); it++) {
        std::cout << std::endl << "-- Scene nr " << it - scenes.begin() << " --" << std::endl;

        if(it->name != "")
            std::cout << "Scene name: " << it->name << std::endl;

        std::cout << "Node count: " << it->node_count << std::endl;

        // output used nodes
        std::cout << "Nodes: ";
        for(uint32_t j = 0; j < it->node_count; j++)
            std::cout << it->nodes[j] << " ";
        std::cout << std::endl;
    }
}


void DASTool::_ListDasNodes(Libdas::DasParser &_parser) {
    auto& nodes = _parser.GetModel().nodes;
    for (auto it = nodes.begin(); it != nodes.end(); it++) {
        std::cout << std::endl << "-- Node nr " << it - nodes.begin() << " --" << std::endl;

        if(it->name != "") std::cout << "Name: " << it->name << std::endl;
        if(it->children_count) {
            std::cout << "Children count: " << it->children_count << std::endl;
            std::cout << "Children: ";
            for(uint32_t j = 0; j < it->children_count; j++)
                std::cout << it->children[j] << " ";
            std::cout << std::endl;
        }
        if(it->mesh != UINT32_MAX)
            std::cout << "Mesh: " << it->mesh << std::endl;
        if(it->skeleton != UINT32_MAX)
            std::cout << "Skeleton: " << it->skeleton << std::endl;

        // output transformation matrix
        std::cout << "Transformation matrix: " << std::endl;
        for(struct { TRS::Matrix4<float>::iterator it; uint32_t i; } s = { it->transform.BeginRowMajor(), 0}; s.it != it->transform.EndRowMajor(); s.it++, s.i++) {
            if(s.i % 4 == 3) 
                std::cout << *s.it << std::endl;
            else std::cout << *s.it << " ";
        }
    }
}


void DASTool::_ListDasMeshes(Libdas::DasParser &_parser) {
    auto& meshes = _parser.GetModel().meshes;
    for (auto it = meshes.begin(); it != meshes.end(); it++) {
        std::cout << std::endl << "-- Mesh nr " << it - meshes.begin() << " --" << std::endl;
        std::cout << "Mesh name: " << it->name << std::endl;
        std::cout << "Mesh primitive count: " << it->primitive_count << std::endl;
        std::cout << "Mesh primitives: ";
        for(uint32_t j = 0; j < it->primitive_count; j++)
            std::cout << it->primitives[j] << " ";
        std::cout << std::endl;

        // for each primitive in mesh output its data
        for(uint32_t j = 0; j < it->primitive_count; j++)
            _ListDasMeshPrimitive(_parser, j, it->primitives[j]);
    }
}


void DASTool::_ListDasMeshPrimitive(Libdas::DasParser &_parser, uint32_t _rel_id, uint32_t _id) {
    const Libdas::DasMeshPrimitive &prim = _parser.GetModel().mesh_primitives[_id];
    std::cout << "---- Primitive nr " << _rel_id << " ----" << std::endl;
    if(prim.index_buffer_id != UINT32_MAX) {
        std::cout << "-- Index buffer id: " << prim.index_buffer_id << std::endl;
        std::cout << "-- Index buffer offset: " << prim.index_buffer_offset << std::endl;
    }
    std::cout << "-- Draw count: " << prim.draw_count << std::endl;
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
    const Libdas::DasMorphTarget &morph = _parser.GetModel().morph_targets[_id];
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
    auto& skeletons = _parser.GetModel().skeletons;
    for (auto it = skeletons.begin(); it != skeletons.end(); it++) {
        std::cout << std::endl << "-- Skeleton nr " << it - skeletons.begin() << " --" << std::endl;
        if(it->name != "") std::cout << "Name: " << it->name << std::endl;
        std::cout << "Parent: " << it->parent << std::endl;
        std::cout << "Joint count: " << it->joint_count << std::endl;

        std::cout << "Joints: ";
        for(uint32_t j = 0; j < it->joint_count; j++)
            std::cout << it->joints[j] << " ";
        std::cout << std::endl;
    }
}


void DASTool::_ListDasSkeletonJoints(Libdas::DasParser &_parser) {
    auto& joints = _parser.GetModel().joints;
    for (auto it = joints.begin(); it != joints.end(); it++) {
        std::cout << std::endl << "-- Skeleton joint nr " << it - joints.begin() << " --" << std::endl;
        // output inverse bind position matrix
        std::cout << "Inverse bind position matrix: " << std::endl;
        for(struct { TRS::Matrix4<float>::iterator it; uint32_t j; } s = { it->inverse_bind_pos.BeginRowMajor(), 0 }; s.it != it->inverse_bind_pos.EndRowMajor(); s.it++, s.j++) {
            if(s.j % 4 == 3) std::cout << *s.it << std::endl;
            else std::cout << *s.it << " ";
        }

        if(it->name != "") std::cout << "Name: " << it->name << std::endl;
        std::cout << "Children count: " << it->children_count << std::endl;
        std::cout << "Children: ";
        for(uint32_t j = 0; j < it->children_count; j++)
            std::cout << it->children[j] << " ";
        std::cout << std::endl;

        std::cout << "Scale: " << it->scale << std::endl;
        std::cout << "Rotation: {" << it->rotation.x << ", " << it->rotation.y << ", " << it->rotation.z << ", " << it->rotation.w <<  "}" << std::endl;
        std::cout << "Translation: {" << it->translation.x << ", " << it->translation.y << ", " << it->translation.z << "}" << std::endl;
    }
}


void DASTool::_ListDasAnimationChannels(Libdas::DasParser &_parser) {
    auto& channels = _parser.GetModel().channels;
    for (auto it = channels.begin(); it != channels.end(); it++) {
        std::cout << std::endl << "-- Animation channel nr " << it - channels.begin() << " --" << std::endl;

        if(it->node_id != UINT32_MAX)
            std::cout << "Referenced node id: " << it->node_id << std::endl;
        else if(it->joint_id != UINT32_MAX)
            std::cout << "Referenced joint id: " << it->joint_id << std::endl;

        // output information about animation target 
        std::cout << "Animation target: ";
        switch(it->target) {
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
        switch(it->interpolation) {
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
        
        std::cout << "Keyframe count: " << it->keyframe_count << std::endl;
        if(it->weight_count)
            std::cout << "Morph target weight count: " << it->weight_count << std::endl;

        std::cout << "Keyframes: ";
        for(uint32_t j = 0; j < it->keyframe_count; j++)
            std::cout << it->keyframes[j] << " ";
        std::cout << std::endl;

        if(it->interpolation == LIBDAS_INTERPOLATION_VALUE_CUBICSPLINE && it->tangents) {
            std::cout << "Keyframe tangents: ";
            const char *tang = it->tangents;
            for(uint32_t j = 0; j < it->keyframe_count; j++) {
                switch(it->target) {
                    case LIBDAS_ANIMATION_TARGET_WEIGHTS:
                        {
                            const float *in = reinterpret_cast<const float*>(tang);
                            tang += it->weight_count * sizeof(float);
                            const float *out = reinterpret_cast<const float*>(tang);
                            tang += it->weight_count * sizeof(float);

                            std::cout << "{{ ";
                            for(uint32_t k = 0; k < it->weight_count; k++) {
                                if(k != it->weight_count - 1)
                                    std::cout << in[k] << ", ";
                                else std::cout << in[k] << "}, ";
                            }
                            for(uint32_t k = 0; k < it->weight_count; k++) {
                                if(k != it->weight_count - 1)
                                    std::cout << out[k] << ", ";
                                else std::cout << out[k] << "}";
                            }
                            std::cout << "} ";
                        }
                        break;

                    case LIBDAS_ANIMATION_TARGET_TRANSLATION:
                        {
                            const TRS::Vector3<float> *in = reinterpret_cast<const TRS::Vector3<float>*>(tang);
                            tang += sizeof(TRS::Vector3<float>);
                            const TRS::Vector3<float> *out = reinterpret_cast<const TRS::Vector3<float>*>(tang);
                            tang += sizeof(TRS::Vector3<float>);
                            std::cout << "{{ " << in->first << ", " << in->second << ", " << in->third << "}, " <<
                                         "{" << out->first << ", " << out->second << ", " << out->third << "}} ";
                        }
                        break;

                    case LIBDAS_ANIMATION_TARGET_ROTATION:
                        {
                            const TRS::Quaternion *in = reinterpret_cast<const TRS::Quaternion*>(tang); 
                            tang += sizeof(TRS::Quaternion);
                            const TRS::Quaternion *out = reinterpret_cast<const TRS::Quaternion*>(tang);
                            tang += sizeof(TRS::Quaternion);
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
        const char *target = it->target_values;
        for(uint32_t j = 0; j < it->keyframe_count; j++) {
            switch(it->target) {
                case LIBDAS_ANIMATION_TARGET_WEIGHTS:
                    std::cout << "{ ";
                    for(uint32_t k = 0; k < it->weight_count; k++) {
                        float w = *reinterpret_cast<const float*>(target);
                        target += sizeof(float);

                        if(k != it->weight_count - 1)
                            std::cout << w << ", ";
                        else std::cout << w << " }";
                    }
                    std::cout << "} ";
                    break;

                case LIBDAS_ANIMATION_TARGET_ROTATION:
                    {
                        const TRS::Quaternion *r = reinterpret_cast<const TRS::Quaternion*>(target);
                        target += sizeof(TRS::Quaternion);
                        std::cout << "{ " << r->x << ", " << r->y << ", " << r->z << ", " << r->w << " } ";
                    }
                    break;

                case LIBDAS_ANIMATION_TARGET_TRANSLATION:
                    {
                        const TRS::Vector3<float> *t = reinterpret_cast<const TRS::Vector3<float>*>(target);
                        target += sizeof(TRS::Vector3<float>);
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
    auto& animations = _parser.GetModel().animations;
    for (auto it = animations.begin(); it != animations.end(); it++) {
        std::cout << std::endl << "-- Animation nr " << it - animations.begin() << " --" << std::endl;
        if(it->name != "") std::cout << "Name: " << it->name << std::endl;
        
        std::cout << "Channel count: " << it->channel_count << std::endl;
        std::cout << "Channels: ";
        for(uint32_t j = 0; j < it->channel_count; j++)
            std::cout << it->channels[j] << " ";
        std::cout << std::endl;
    }
}


void DASTool::_ListDas(const std::string &_input_file) {
    Libdas::DasParser parser(_input_file);
    parser.Parse();
    
    const Libdas::DasProperties &props = parser.GetModel().props;
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

        case USAGE_FLAG_EMBED_TEXTURE:
            m_embedded_textures.push_back(_arg);
            break;

        case USAGE_FLAG_OUT_FILE:
            m_out_file = _arg;
            break;

        case USAGE_FLAG_LOD:
            m_lod = static_cast<uint32_t>(std::stoi(_arg));
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
        if(_opts[i] == "--author") {
            m_flags |= USAGE_FLAG_AUTHOR;
            info_flag = USAGE_FLAG_AUTHOR; 
            skip_it = true;
        } 
        else if(_opts[i] == "--copyright") {
            m_flags |= USAGE_FLAG_COPYRIGHT;
            info_flag = USAGE_FLAG_COPYRIGHT; 
            skip_it = true;
        }
        else if(_opts[i] == "--embed-texture") {
            m_flags |= USAGE_FLAG_EMBED_TEXTURE;
            info_flag = USAGE_FLAG_EMBED_TEXTURE; 
            skip_it = true;
        }
        else if(_opts[i] == "--model") {
            m_flags |= USAGE_FLAG_MODEL;
            info_flag = USAGE_FLAG_MODEL; 
            skip_it = true;
        }
        else if (_opts[i] == "-L" || _opts[i] == "--lod") {
            m_flags |= USAGE_FLAG_LOD;
            info_flag = USAGE_FLAG_LOD;
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
        if((m_flags & USAGE_FLAG_AUTHOR) == USAGE_FLAG_AUTHOR) {
            std::cerr << "Invalid use of author flag in listing mode" << std::endl;
            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_KEYWORD);
        }
        else if((m_flags & USAGE_FLAG_COPYRIGHT) == USAGE_FLAG_COPYRIGHT) {
            std::cerr << "Invalid use of copyright specifier flag in listing mode" << std::endl;
            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_KEYWORD);
        }
        else if((m_flags & USAGE_FLAG_EMBED_TEXTURE) == USAGE_FLAG_EMBED_TEXTURE) {
            std::cerr << "Invalid use of embed texture flag in listing mode" << std::endl;
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
    if (ext == "stl")
        _ConvertSTL(_input_file);
    else if (ext == "obj")
        _ConvertWavefrontObj(_input_file);
    else if (ext == "gltf")
        _ConvertGLTF(_input_file);
    else if (ext == "glb")
        _ConvertGLB(_input_file);
    else if (ext == "fbx")
        _ConvertFBX(_input_file);
    else if (ext == "das")
        _ConvertDAS(_input_file);

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


void DASTool::Validate(const std::string &_input_file) {
    Libdas::DasParser parser(_input_file);
    parser.Parse(true);

    Libdas::DasValidator validator(parser.GetModel());

    // warnings
    while(!validator.IsWarningStackEmpty())
        std::cout << validator.PopWarningStack() << std::endl;

    // errors
    while(!validator.IsErrorStackEmpty())
        std::cerr << validator.PopErrorStack() << std::endl;
}


// main method
int main(int argc, char *argv[]) {
    DASTool tool;
    if(argc < 3 || (std::string(argv[1]) != "convert" && std::string(argv[1]) != "list" && std::string(argv[1]) != "validate")) {
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
    else if(std::string(argv[1]) == "validate")
        tool.Validate(argv[2]);

    return 0;
}
