// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: LodGenerator.h - class header for generating multiple LODs
// author: Karl-Mihkel Ott

#ifndef LOD_GENERATOR_H
#define LOD_GENERATOR_H

#ifdef LOD_GENERATOR_CPP
	#include <cstdint>
	#include <vector>
	#include <array>
	#include <string>
	#include <unordered_set>
	#include <queue>
	#include <algorithm>
	#include <map>
	#include <iostream>

	#define SQ(x) (x*x)

	#include "trs/Vector.h"
	#include "trs/Points.h"
	#include "trs/Quaternion.h"
	#include "trs/Matrix.h"
	
	#include "das/Api.h"
	#include "das/Hash.h"
	#include "das/DasStructures.h"
#endif

namespace Libdas {
	
	struct Edge {
		TRS::Matrix4<float> Q;
		TRS::Vector3<float> new_pos;
		uint32_t first_vertex = 0;
		uint32_t second_vertex = 0;
		float edge_error = 0.f;

		bool operator==(const Edge& e2) const {
			return first_vertex == e2.first_vertex && second_vertex == e2.second_vertex;
		}

		struct CompareEdge {
			bool operator()(const Edge& e1, const Edge& e2) const {
				return e2.edge_error < e1.edge_error;
			}
		};
	};

	class LIBDAS_API LodGenerator {
		private:
			std::list<uint32_t> m_indices;
			std::vector<TRS::Vector3<float>> m_vertices;

			// generated vertices and indices go here
			std::list<uint32_t> m_generated_indices;
			std::vector<TRS::Vector3<float>> m_generated_vertices;

			std::vector<Edge> m_edges;

			// first: vertex index
			// second: neighbour's index
			std::multimap<uint32_t, uint32_t> m_neighbours;
			std::vector<TRS::Matrix4<float>> m_errors;

		private:
			void _FindUniqueEdges();
			void _FindVertexNeighbours();
			std::vector<Edge> _RemoveDuplicateEdges();

			// calculate error quadric matrix for single vertex
			TRS::Matrix4<float> _CalculateVertexErrorQuadric(uint32_t _index);

			void _CalculateEdgeErrors(
				Edge& _edge, 
				const std::vector<TRS::Vector3<float>>& _vertices,
				const std::vector<TRS::Matrix4<float>>& _errors);
			void _RemoveInvalidFaces(uint32_t _first, uint32_t _second, std::vector<Edge>& _edges, float& _facec);

		public:
			LodGenerator(const uint32_t* _indices, 
						 const TRS::Vector3<float>* _vertices,
						 uint32_t _draw_count);

			void Simplify(float _t);

			std::vector<uint32_t> GetLodIndices();
			std::vector<TRS::Vector3<float>> GetLodVertices();
 	};
}

#endif
