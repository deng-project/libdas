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
    }


    void GLTFCompiler::Compile(const GLTFRoot &_root, const DasProperties &_props, const std::string &_out_file) {
        // check if new file should be opened
        if(_out_file != "")
            NewFile(_out_file);

        _CheckAndSupplementProperties(const_cast<GLTFRoot&>(_root), const_cast<DasProperties&>(_props));
        InitialiseFile(_props);

        // write buffers to file
        std::vector<DasBuffer> buffers = _MakeBuffers(_root);
        for(auto it = buffers.begin(); it != buffers.end(); it++)
            WriteBuffer(*it);

        // write models to file
        std::vector<DasModel> models = _MakeModels(_root);
        for(auto it = models.begin(); it != models.end(); it++)
            WriteModel(*it);

        // write animations to file
        std::vector<DasAnimation> animation = _MakeAnimations(_root);
        for(auto it = animation.begin(); it != animations.end(); it++)
            WriteAnimation(*it);

        // write skeletons to file
    }
}
