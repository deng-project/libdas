// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasValidator.h - DAS file validation class header
// author: Karl-Mihkel Ott

#ifndef DAS_VALIDATOR_H
#define DAS_VALIDATOR_H

#ifdef DAS_VALIDATOR_CPP
    #include <string>
    #include <stack>
    #include <fstream>
    #include <vector>
    #include <unordered_map>
    #include <any>

    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <Quaternion.h>

    #include <Api.h>
    #include <ErrorHandlers.h>
    #include <AsciiStreamReader.h>
    #include <AsciiLineReader.h>
    #include <DasStructures.h>
    #include <DasReaderCore.h>
    #include <DasParser.h>
#endif

namespace Libdas {

    class DasValidator {
        private:
            DasParser &m_parser;
            std::stack<std::string> m_warning_stack;
            std::stack<std::string> m_error_stack;

        private:
            void _VerifyProperties();
            void _VerifyMeshPrimitives();
            void _VerifyMorphTargets();
            void _FindRecursiveNodes();
            void _VerifyScenes();
            void _FindRecursiveSkeletonJoints();
            void _VerifySkeleton();

        public:
            DasValidator(DasParser &_parser, bool _validate = true);
            DasValidator(DasValidator &&_val) noexcept;

            void Validate();
    };
}

#endif
