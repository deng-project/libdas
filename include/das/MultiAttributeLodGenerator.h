// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: MultiAttributeLodGenerator.h - class header for generating multiple LODs for multiattribute vertices
// author: Karl-Mihkel Ott

#ifndef MULTI_ATTRIBUTE_LOD_GENERATOR_H
#define MULTI_ATTRIBUTE_LOD_GENERATOR_H

#define MULTI_ATTRIBUTE_LOD_GENERATOR_CPP

#ifdef MULTI_ATTRIBUTE_LOD_GENERATOR_CPP
	#include <map>
	#include <list>
	#include <algorithm>
	#include <array>
	#include <iostream>
	#include <iterator>

	#include <trs/MatrixN.h>

	#define SQ(x) ((x)*(x))
#endif


namespace Libdas {

	typedef std::vector<std::vector<float>> MatrixN;
	typedef std::vector<float> VectorN;

	struct ErrorMetric {
		TRS::MatrixN<float> A;
		TRS::VectorN<float> b;
		float c = 0.0f;
	};


	struct MultiAttributeEdge {
		TRS::VectorN<float> substitudeVertex;

		uint32_t uFirstVertex = 0;
		uint32_t uSecondVertex = 0;
		float fEdgeError = 0.f;

		bool operator==(const MultiAttributeEdge& edge2) const {
			return uFirstVertex == edge2.uFirstVertex && uSecondVertex == edge2.uSecondVertex;
		}

		struct CompareEdge {
			bool operator()(const MultiAttributeEdge& e1, const MultiAttributeEdge& e2) {
				return e2.fEdgeError < e1.fEdgeError;
			}
		};
	};

	class MultiAttributeLodGenerator {
		private:
			std::vector<TRS::VectorN<float>> m_vertices;
			std::list<uint32_t> m_indices;

			std::vector<MultiAttributeEdge> m_edges;
			std::multimap<uint32_t, uint32_t> m_neighbours;
			
			std::list<uint32_t> m_generatedIndices;
			std::vector<TRS::VectorN<float>> m_generatedVertices;
			
			std::vector<ErrorMetric> m_errors;

		private:
			bool _IsTriangle(uint32_t _uSecond, uint32_t _uThird);
			void _FindUniqueEdges();
			void _FindVertexNeighbours();
			std::vector<MultiAttributeEdge> _RemoveDuplicateEdges();
			ErrorMetric _CalculateVertexErrorQuadric(uint32_t _uIndex);
			void _CalculateEdgeError(
				MultiAttributeEdge& _edge,
				const std::vector<TRS::VectorN<float>>& _vertices,
				const std::vector<ErrorMetric>& _errors);
			void _RemoveInvalidFaces(
				uint32_t _uSubstitudedIndex,
				uint32_t _uRemovedIndex,
				std::vector<MultiAttributeEdge>& _errors,
				float& _fFaceCount);


		public:
			MultiAttributeLodGenerator(const std::vector<std::pair<float*, uint32_t>>& _vertices, uint32_t* _uIndices, uint32_t _uDrawCount);
			void Simplify(float _t);

			std::vector<uint32_t> GetLodIndices();
			inline std::vector<TRS::VectorN<float>>& GetLodVertices() {
				return m_generatedVertices;
			}
	};
}

#endif