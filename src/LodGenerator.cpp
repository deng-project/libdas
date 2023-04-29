// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: LodGenerator.cpp - class header for generating multiple LODs
// author: Karl-Mihkel Ott

#define LOD_GENERATOR_CPP
#include "das/LodGenerator.h"

using namespace std;

namespace Libdas {

	LodGenerator::LodGenerator(const uint32_t* _indices, const TRS::Vector3<float>* _vertices, uint32_t _draw_count) {
		// copy draw data to std::list
		uint32_t max_vertex = 0;
		for (uint32_t i = 0; i < _draw_count; i++) {
			m_indices.push_back(_indices[i]);

			if (_indices[i] > max_vertex)
				max_vertex = _indices[i];
		}

		for (uint32_t i = 0; i < max_vertex+1; i++) {
			m_vertices.push_back(_vertices[i]);
		}

		_FindUniqueEdges();
		_FindVertexNeighbours();
		_FlagDiscontinuities();

		// calculate vertex errors
		for (size_t i = 0; i < m_vertices.size(); i++) {
			m_errors.push_back(_CalculateVertexErrorQuadric(static_cast<uint32_t>(i)));
		}

		_AdjustVertexErrorQuadrics();

		// calculate edge contraction errors
		for (size_t i = 0; i < m_edges.size(); i++) {
			_CalculateEdgeErrors(m_edges[i], m_vertices, m_errors);
		}
	}


	bool LodGenerator::_IsTriangle(uint32_t _second, uint32_t _third) {
		pair<multimap<uint32_t, uint32_t>::iterator, multimap<uint32_t, uint32_t>::iterator> 
			res = m_neighbours.equal_range(_second);

		for (auto it = res.first; it != res.second; it++) {
			if (_third == it->second) {
				return true;
			}
		}

		return false;
	}

	
	void LodGenerator::_FindUniqueEdges() {
		int counter = 0;
		list<uint32_t>::iterator a, b, c;
		for (auto it = m_indices.begin(); it != m_indices.end(); it++) {
			switch (counter) {
				case(0):
					a = it;
					counter++;
					break;
				
				case(1):
					counter++;
					b = it;
					break;
				
				case(2):
					c = it;
					counter = 0;

					Edge e1, e2, e3;
					e1.first_vertex = *a;
					e1.second_vertex = *b;

					if (*a > *b) {
						e1.first_vertex = *b;
						e1.second_vertex = *a;
					}

					e2.first_vertex = *a;
					e2.second_vertex = *c;

					if (*a > *c) {
						e1.first_vertex = *c;
						e1.second_vertex = *a;
					}

					e3.first_vertex = *b;
					e3.second_vertex = *c;

					if (*b > *c) {
						e1.first_vertex = *c;
						e1.second_vertex = *b;
					}

					m_edges.push_back(e1);
					m_edges.push_back(e2);
					m_edges.push_back(e3);
					break;
			}
		}

		m_edges = _RemoveDuplicateEdges();
	}


	void LodGenerator::_FlagDiscontinuities() {
		for (size_t i = 0; i < m_edges.size(); i++) {
			pair<multimap<uint32_t, uint32_t>::iterator, multimap<uint32_t, uint32_t>::iterator>
				neighbour_range = m_neighbours.equal_range(m_edges[i].first_vertex);

			int counter = 0;
			for (auto it = neighbour_range.first; it != neighbour_range.second; it++) {
				if (it->second == m_edges[i].second_vertex)
					counter++;
			}

			if (counter < 2) {
				m_edges[i].is_discontinuity = true;
			}
			else 
				m_edges[i].is_discontinuity = false;
		}
	}


	void LodGenerator::_FindVertexNeighbours() {
		for (size_t i = 0; i < m_vertices.size(); i++) {
			for (size_t j = 0; j < m_edges.size(); j++) {
				if (i == m_edges[j].first_vertex)
					m_neighbours.insert(make_pair(static_cast<uint32_t>(i), m_edges[j].second_vertex));
				if (i == m_edges[j].second_vertex)
					m_neighbours.insert(make_pair(static_cast<uint32_t>(i), m_edges[j].first_vertex));
			}
		}
	}

	// utility method to remove all duplicate edges in vector
	vector<Edge> LodGenerator::_RemoveDuplicateEdges() {
		vector<Edge> no_duplicates;

		for (size_t i = 0; i < m_edges.size(); i++) {
			bool is_dup = false;
			for (size_t j = i + 1; j < m_edges.size(); j++) {
				if (m_edges[i] == m_edges[j]) {
					is_dup = true;
					break;
				}
			}

			if (!is_dup)
				no_duplicates.push_back(m_edges[i]);
		}

		return no_duplicates;
	}

	TRS::Matrix4<float> LodGenerator::_CalculateVertexErrorQuadric(uint32_t _index) {
		TRS::Matrix4<float> Q_matrix;
		pair <multimap<uint32_t, uint32_t>::iterator, multimap<uint32_t, uint32_t>::iterator> res = m_neighbours.equal_range(_index);
		
		// find all triangles this vertex has and add its errors to it
		for (auto it = res.first; it != res.second; it++) {
			for (auto it2 = it; it2 != res.second; it2++) {
				if (it->second == it2->second)
					continue;

				if (_IsTriangle(it->second, it2->second)) {
					TRS::Vector3<float> n = TRS::Vector3<float>::Cross(
						m_vertices[it2->second] - m_vertices[_index],
						m_vertices[it->second] - m_vertices[_index]);
					n.Normalise();

					const float a = n.first, b = n.second, c = n.third;
					const float d = -(m_vertices[_index] * n);

					Q_matrix += TRS::Matrix4<float>{
						{ SQ(a), a* b, a* c, a* d },
						{ a * b, SQ(b), b * c, b * d },
						{ a * c, b * c, SQ(c), c * d },
						{ a * d, b * d, c * d, SQ(d) }
					};
				}
			}
		}

		return Q_matrix;
	}

	// adjust these vertex quadrics which are part of discontinuous edges
	void LodGenerator::_AdjustVertexErrorQuadrics() {
		for (size_t i = 0; i < m_edges.size(); i++) {
			if (m_edges[i].is_discontinuity) {
				pair<multimap<uint32_t, uint32_t>::iterator, multimap<uint32_t, uint32_t>::iterator>
					res = m_neighbours.equal_range(m_edges[i].first_vertex);

				for (auto it = res.first; it != res.second; it++) {
					if (_IsTriangle(it->second, m_edges[i].second_vertex)) {
						TRS::Vector3<float> n1 = TRS::Vector3<float>::Cross(
							m_vertices[m_edges[i].second_vertex] - m_vertices[m_edges[i].first_vertex],
							m_vertices[it->second] - m_vertices[m_edges[i].first_vertex]);
						n1.Normalise();

						TRS::Vector3<float> n2 = TRS::Vector3<float>::Cross(
							m_vertices[m_edges[i].second_vertex] - m_vertices[m_edges[i].first_vertex], n1);
						n2.Normalise();

						const float a = n2.first, b = n2.second, c = n2.third;
						const float d = -(m_vertices[m_edges[i].first_vertex] * n2);

						const TRS::Matrix4<float> Q_error = {
							{ SQ(a), a * b, a * c, a * d },
							{ a * b, SQ(b), b * c, b * d },
							{ a * c, b * c, SQ(c), c * d },
							{ a * d, b * d, c * d, SQ(d) }
						};

						m_errors[m_edges[i].first_vertex] += Q_error;
						m_errors[m_edges[i].second_vertex] += Q_error;
					}
				}
			}
		}
	}


	void LodGenerator::_CalculateEdgeErrors(
		Edge& _edge, 
		const vector<TRS::Vector3<float>>& _vertices,
		const vector<TRS::Matrix4<float>>& _errors) 
	{
		_edge.Q = _errors[_edge.first_vertex] + _errors[_edge.second_vertex];
		_edge.new_pos = (_vertices[_edge.first_vertex] + _vertices[_edge.second_vertex]) / 2.f;

		TRS::Vector4<float> mid(_edge.new_pos.first, _edge.new_pos.second, _edge.new_pos.third, 1.f);
		TRS::Vector4<float> tmp = _edge.Q * mid;
		
		_edge.edge_error = mid * tmp;
	}

	void LodGenerator::_RemoveInvalidFaces(uint32_t _first, uint32_t _second, vector<Edge>& _edges) {
		bool fallback = false;
		uint32_t a, b, c;
		int counter = 0;

		for (auto it = m_generated_indices.begin(); it != m_generated_indices.end(); it++) {
			if (fallback) {
				it--;
				fallback = false;
			}

			switch (counter) {
				case(0):
					a = *it;
					counter++;
					break;

				case(1):
					b = *it;
					counter++;
					break;

				case(2):
				{
					c = *it;
					counter = 0;

					// if the current face contains given edge indices
					if ((_first == a || _first == b || _first == c) && (_second == a || _second == b || _second == c)) {
						for (size_t i = 0; i < _edges.size(); ) {
							if ((_edges[i].first_vertex == a || _edges[i].first_vertex == b || _edges[i].first_vertex == c) &&
								(_edges[i].second_vertex == a || _edges[i].second_vertex == b || _edges[i].second_vertex == c))
							{
								if (_edges[i].first_vertex != _first && _edges[i].second_vertex != _first)
									_edges.erase(_edges.begin() + i);
								else if (_edges[i].first_vertex == _first && _edges[i].second_vertex == _first)
									_edges.erase(_edges.begin() + i);
								else 
									i++;
							}
							else
								i++;
						}

						it--;
						it--;
						m_generated_indices.erase(it++);
						m_generated_indices.erase(it++);
						m_generated_indices.erase(it++);

						if (it != m_generated_indices.begin())
							it--;
						else
							fallback = true;
					}
					break;
				}
			}
		}

		for (list<uint32_t>::iterator it = m_generated_indices.begin(); it != m_generated_indices.end(); it++) {
			if (*it == _second)
				*it = _first;
		}
	}

	void LodGenerator::Simplify(float _t) {
		float facec = static_cast<float>(m_indices.size() / 3);
		const float max_faces = facec * _t;

		// copy relevant values
		vector<TRS::Matrix4<float>> errors = m_errors;
		vector<Edge> edges = m_edges;
		multimap<uint32_t, uint32_t> neighbours = m_neighbours;
		
		make_heap(edges.begin(), edges.end(), Edge::CompareEdge());

		m_generated_indices = m_indices;
		m_generated_vertices = m_vertices;

		while (facec > max_faces) {
			pop_heap(edges.begin(), edges.end(), Edge::CompareEdge());
			Edge removed_edge = edges.back();

			errors[removed_edge.first_vertex] = removed_edge.Q;
			m_generated_vertices[removed_edge.first_vertex] = removed_edge.new_pos;

			// remove invalid vertices
			for (uint32_t i = 0; i < static_cast<uint32_t>(m_generated_vertices.size()); i++) {
				int count = 0;

				pair<multimap<uint32_t, uint32_t>::iterator, multimap<uint32_t, uint32_t>::iterator> ret = neighbours.equal_range(i);

				for (auto it = ret.first; it != ret.second;) {
					if (i != removed_edge.first_vertex) {
						if (it->second == removed_edge.second_vertex && count == 0) {
							it->second = removed_edge.first_vertex;
							count++;
							it++;
						}
						else if (it->second == removed_edge.second_vertex) {
							neighbours.erase(it++);
						}
						else it++;
					}
					else {
						if (it->second == removed_edge.second_vertex)
							neighbours.erase(it++);
						else it++;
					}
				}
			}

			for (uint32_t i = 0; i < static_cast<uint32_t>(m_generated_vertices.size()); i++) {
				int count = 0;

				pair<multimap<uint32_t, uint32_t>::iterator, multimap<uint32_t, uint32_t>::iterator> ret = neighbours.equal_range(i);
				
				for (auto it = ret.first; it != ret.second;) {
					if (it->second == removed_edge.first_vertex && count == 0) {
						count++;
						it++;
					}
					else if (it->second == removed_edge.first_vertex)
						neighbours.erase(it++);
					else it++;
				}
			}

			// remove the edge with minimal error
			edges.pop_back();

			facec -= 2.f;

			neighbours.erase(removed_edge.second_vertex);

			_RemoveInvalidFaces(removed_edge.first_vertex, removed_edge.second_vertex, edges);

			// update edge vector, so any edge that was connected to the removed edge has it's error recalculated
			// and is only connected to the remaining vertex
			for (size_t i = 0; i < edges.size(); i++) {
				if (edges[i].second_vertex == removed_edge.second_vertex) {
					edges[i].second_vertex = removed_edge.first_vertex;
				}
				else if (edges[i].first_vertex == removed_edge.second_vertex) {
					edges[i].first_vertex = removed_edge.first_vertex;
				}
			}

			for (size_t i = 0; i < edges.size(); i++) {
				if (edges[i].first_vertex == removed_edge.first_vertex || edges[i].second_vertex == removed_edge.first_vertex)
					_CalculateEdgeErrors(edges[i], m_generated_vertices, errors);
			}

			make_heap(edges.begin(), edges.end(), Edge::CompareEdge());
		}
	}

	vector<uint32_t> LodGenerator::GetLodIndices() {
		vector<uint32_t> idx_vector;
		idx_vector.reserve(m_generated_indices.size());

		for (uint32_t idx : m_generated_indices)
			idx_vector.push_back(idx);

		return idx_vector;
	}

	vector<TRS::Vector3<float>> LodGenerator::GetLodVertices() {
		return m_generated_vertices;
	}
}