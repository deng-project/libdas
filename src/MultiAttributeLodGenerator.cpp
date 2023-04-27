// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: MultiAttributeLodGenerator.cpp - class implementation for generating multiple LODs for multiattribute vertices
// author: Karl-Mihkel Ott

#define MULTI_ATTRIBUTE_LOD_GENERATOR_CPP
#include <das/MultiAttributeLodGenerator.h>

using namespace std;

namespace Libdas {
	
	MultiAttributeLodGenerator::MultiAttributeLodGenerator(const vector<pair<float*, uint32_t>>& _attrs, uint32_t* _pIndices, uint32_t uDrawCount) {
		// put data into appropriate data structures
		// 1. indices
		uint32_t uMaxIndex = 0;
		for (uint32_t i = 0; i < uDrawCount; i++) {
			m_indices.push_back(_pIndices[i]);

			if (m_indices.back() > uMaxIndex)
				uMaxIndex = m_indices.back();
		}

		// 2. vertices
		size_t uDimentions = 0;
		for (auto it = _attrs.begin(); it != _attrs.end(); it++) {
			uDimentions += it->second;
		}
		
		for (uint32_t i = 0; i <= uMaxIndex; i++) {
			m_vertices.emplace_back();
			for (auto it = _attrs.begin(); it != _attrs.end(); it++) {
				size_t uOffset = static_cast<size_t>(it->second * i);
				for (uint32_t j = 0; j < it->second; j++)
					m_vertices.back().push_back(it->first[uOffset + j]);
			}
		}

		_FindUniqueEdges();
		_FindVertexNeighbours();

		for (size_t i = 0; i < m_vertices.size(); i++) {
			m_errors.push_back(_CalculateVertexErrorQuadric(static_cast<uint32_t>(i)));
		}

		for (size_t i = 0; i < m_edges.size(); i++) {
			_CalculateEdgeError(m_edges[i], m_vertices, m_errors);
		}
	}

	bool MultiAttributeLodGenerator::_IsTriangle(uint32_t _uSecond, uint32_t _uThird) {
		pair<multimap<uint32_t, uint32_t>::iterator, multimap<uint32_t, uint32_t>::iterator>
			res = m_neighbours.equal_range(_uSecond);

		for (auto it = res.first; it != res.second; it++) {
			if (_uThird == it->second)
				return true;
		}

		return false;
	}

	void MultiAttributeLodGenerator::_FindUniqueEdges() {
		uint32_t uCounter = 0;
		uint32_t a = 0, b = 0, c = 0;

		for (auto it = m_indices.begin(); it != m_indices.end(); it++) {
			switch (uCounter) {
				case (0):
					a = *it;
					uCounter++;
					break;

				case (1):
					b = *it;
					uCounter++;
					break;

				case (2):
				{
					c = *it;
					uCounter = 0;

					MultiAttributeEdge e1, e2, e3;
					
					e1.uFirstVertex = a;
					e1.uSecondVertex = b;
					
					
					if (a > b) {
						e1.uFirstVertex = b;
						e1.uSecondVertex = a;
					}
					
					e2.uFirstVertex = a;
					e2.uSecondVertex = c;
					
					if (a > c) {
						e1.uFirstVertex = c;
						e1.uSecondVertex = a;
					}
					
					e3.uFirstVertex = b;
					e3.uSecondVertex = c;
					
					if (b > c) {
						e1.uFirstVertex = c;
						e1.uSecondVertex = b;
					}

					m_edges.push_back(e1);
					m_edges.push_back(e2);
					m_edges.push_back(e3);
					break;
				}
			}
		}

		m_edges = _RemoveDuplicateEdges();
	}

	void MultiAttributeLodGenerator::_FindVertexNeighbours() {
		for (uint32_t i = 0; i < static_cast<uint32_t>(m_vertices.size()); i++) {
			for (size_t j = 0; j < m_edges.size(); j++) {
				if (i == m_edges[j].uFirstVertex)
					m_neighbours.insert(make_pair(i, m_edges[j].uSecondVertex));
				if (i == m_edges[j].uSecondVertex)
					m_neighbours.insert(make_pair(i, m_edges[j].uFirstVertex));
			}
		}
	}


	vector<MultiAttributeEdge> MultiAttributeLodGenerator::_RemoveDuplicateEdges() {
		vector<MultiAttributeEdge> noDuplicates;

		for (size_t i = 0; i < m_edges.size(); i++) {
			bool isDuplicate = false;
			for (size_t j = i + 1; j < m_edges.size(); j++) {
				if (m_edges[i] == m_edges[j]) {
					isDuplicate = true;
					break;
				}
			}

			if (!isDuplicate)
				noDuplicates.push_back(m_edges[i]);
		}

		return noDuplicates;
	}

	ErrorMetric MultiAttributeLodGenerator::_CalculateVertexErrorQuadric(uint32_t _uIndex) {
		ErrorMetric metric;
		metric.A = TRS::MatrixN<float>(m_vertices[0].size());
		metric.b.resize(m_vertices[0].size());

		pair<multimap<uint32_t, uint32_t>::iterator, multimap<uint32_t, uint32_t>::iterator> 
			res = m_neighbours.equal_range(_uIndex);

		for (auto it = res.first; it != res.second; it++) {
			for (auto it2 = it; it2 != res.second; it2++) {
				if (it->second == it2->second)
					continue;

				if (_IsTriangle(it->second, it2->second)) {
					TRS::VectorN<float>& p = m_vertices[it->first];
					TRS::VectorN<float>& q = m_vertices[it->second];
					TRS::VectorN<float>& r = m_vertices[it2->second];
					
					TRS::VectorN<float> e1 = (q - p);
					e1.Normalise();
					TRS::VectorN<float> e2 = r - p - (e1 * (e1 * (r - p)));
					e2.Normalise();

					metric.A += (TRS::MatrixN<float>::MakeIdentity(e1.size()) - e1.ExpandToMatrix() - e2.ExpandToMatrix());
					metric.b = metric.b + ((e1 * (p * e1)) + (e2 * (p * e2)) - p);
					metric.c += p * p - SQ(p * e1) - SQ(p * e2);
				}
			}
		}

		return metric;
	}


	void MultiAttributeLodGenerator::_CalculateEdgeError(
		MultiAttributeEdge& _edge, 
		const vector<TRS::VectorN<float>>& _vertices,
		const vector<ErrorMetric>& _errors) 
	{
		const array<const TRS::MatrixN<float>*, 2> aQuadrics = 
			{ &_errors[_edge.uFirstVertex].A, &_errors[_edge.uSecondVertex].A };

		const array<const TRS::VectorN<float>*, 2> bQuadrics = 
			{ &_errors[_edge.uFirstVertex].b, &_errors[_edge.uSecondVertex].b };

		const array<float, 2> cScalars = 
			{ _errors[_edge.uFirstVertex].c, _errors[_edge.uSecondVertex].c };

		const float a1Determinant = aQuadrics[0]->Determinant();
		const float a2Determinant = aQuadrics[1]->Determinant();
		if ((a1Determinant > 0.000001 || a1Determinant < 0.000001) && (a2Determinant > 0.000001 || a2Determinant < 0.000001)) {
			const TRS::MatrixN<float> edgeA = *aQuadrics[0] + *aQuadrics[1];
			const TRS::VectorN<float> edgeB = *bQuadrics[0] + *bQuadrics[1];

			_edge.substitudeVertex = -edgeA.Inverse() * edgeB;
		}
		else {
			_edge.substitudeVertex = _vertices[_edge.uFirstVertex];
		}

		const float fError1 =
			(_edge.substitudeVertex * (*aQuadrics[0]) * _edge.substitudeVertex) +
			(((*bQuadrics[0]) * 2.f * _edge.substitudeVertex) + cScalars[0]);

		const float fError2 =
			(_edge.substitudeVertex * (*aQuadrics[1]) * _edge.substitudeVertex) +
			(((*bQuadrics[1]) * 2.f * _edge.substitudeVertex) + cScalars[1]);


		_edge.fEdgeError = fError1 + fError2;

		// clamp
		if (_edge.fEdgeError < 0.f)
			_edge.fEdgeError = 0.f;
	}


	void MultiAttributeLodGenerator::_RemoveInvalidFaces(
		uint32_t _uSubstitudedIndex,
		uint32_t _uRemovedIndex,
		vector<MultiAttributeEdge>& _edges)
	{
		bool bFallback = false;
		uint32_t a, b, c;
		int counter = 0;

		cout << "[uSubstitudedIndex occurances] ";
		for (auto it2 = m_generatedIndices.begin(); it2 != m_generatedIndices.end(); it2++) {
			if (*it2 == _uSubstitudedIndex)
				cout << counter << ' ';
			counter++;
		}
		cout << '\n';

		counter = 0;
		cout << "[uRemovedIndex occurances] ";
		for (auto it = m_generatedIndices.begin(); it != m_generatedIndices.end(); it++) {
			if (*it == _uRemovedIndex)
				cout << counter << ' ';

			counter++;
		}
		cout << '\n';

		counter = 0;

		for (auto it = m_generatedIndices.begin(); it != m_generatedIndices.end(); it++) {
			if (bFallback) {
				it--;
				bFallback = false;
			}

			switch (counter) {
				case (0):
					a = *it;
					counter++;
					break;

				case (1):
					b = *it;
					counter++;
					break;

				case (2):
				{
					c = *it;
					counter = 0;

					// check if the current face contains given indices
					if ((a == _uSubstitudedIndex || b == _uSubstitudedIndex || c == _uSubstitudedIndex) &&
						(a == _uRemovedIndex || b == _uRemovedIndex || c == _uRemovedIndex))
					{
						for (size_t i = 0; i < _edges.size(); i++) {
							if ((_edges[i].uFirstVertex == a || _edges[i].uFirstVertex == b || _edges[i].uFirstVertex == c) &&
								(_edges[i].uSecondVertex == a || _edges[i].uSecondVertex == b || _edges[i].uSecondVertex == c))
							{
								if (_edges[i].uFirstVertex != _uSubstitudedIndex && _edges[i].uSecondVertex != _uSubstitudedIndex)
									_edges.erase(_edges.begin() + i);
								else if (_edges[i].uFirstVertex == _uSubstitudedIndex && _edges[i].uSecondVertex == _uSubstitudedIndex)
									_edges.erase(_edges.begin() + i);
								else i++;
							}
							else 
								i++;
						}

						it--;
						it--;
						m_generatedIndices.erase(it++);
						m_generatedIndices.erase(it++);
						m_generatedIndices.erase(it++);

						if (it != m_generatedIndices.begin())
							it--;
						else
							bFallback = true;
					}
					break;
				}
			}
		}

		// correct indices
		for (list<uint32_t>::iterator it = m_generatedIndices.begin(); it != m_generatedIndices.end(); it++) {
			if (*it == _uRemovedIndex)
				*it = _uSubstitudedIndex;
		}
	}


	void MultiAttributeLodGenerator::Simplify(float _fRate) {
		float fFaceCount = static_cast<float>(m_indices.size() / 3);
		const float fMaxFaces = fFaceCount * _fRate;

		// copy values
		vector<ErrorMetric> errors = m_errors;
		vector<MultiAttributeEdge> edges = m_edges;
		multimap<uint32_t, uint32_t> neighbours = m_neighbours;

		m_generatedVertices = m_vertices;
		m_generatedIndices = m_indices;

		make_heap(edges.begin(), edges.end(), MultiAttributeEdge::CompareEdge());

		while (fFaceCount > fMaxFaces) {
			pop_heap(edges.begin(), edges.end(), MultiAttributeEdge::CompareEdge());
			MultiAttributeEdge removedEdge = edges.back();

			m_generatedVertices[removedEdge.uFirstVertex] = removedEdge.substitudeVertex;

			// remove invalid vertices
			for (uint32_t i = 0; i < static_cast<uint32_t>(m_generatedVertices.size()); i++) {
				bool bFirstRemovedNeighbourChanged = false;

				pair<multimap<uint32_t, uint32_t>::iterator, multimap<uint32_t, uint32_t>::iterator>
					res = neighbours.equal_range(i);

				for (auto it = res.first; it != res.second;) {
					if (i != removedEdge.uFirstVertex) {
						if (it->second == removedEdge.uSecondVertex && !bFirstRemovedNeighbourChanged) {
							it->second = removedEdge.uFirstVertex;
							bFirstRemovedNeighbourChanged = true;
							it++;
						}
						else if (it->second == removedEdge.uSecondVertex) {
							neighbours.erase(it++);
						}
						else it++;
					}
					else {
						if (it->second == removedEdge.uSecondVertex) {
							neighbours.erase(it++);
						}
						else it++;
					}
				}
			}

			for (uint32_t i = 0; i < static_cast<uint32_t>(m_generatedVertices.size()); i++) {
				bool bSkippedModifiedVertex = false;

				pair<multimap<uint32_t, uint32_t>::iterator, multimap<uint32_t, uint32_t>::iterator>
					res = neighbours.equal_range(i);

				for (auto it = res.first; it != res.second;) {
					if (it->second == removedEdge.uFirstVertex && !bSkippedModifiedVertex) {
						bSkippedModifiedVertex = true;
						it++;
					}
					else if (it->second == removedEdge.uFirstVertex)
						neighbours.erase(it++);
					else it++;
				}
			}

			// remove the edge with minimal error
			edges.pop_back();
			fFaceCount -= 2.f;
			neighbours.erase(removedEdge.uSecondVertex);

			// remove any invalid faces
			_RemoveInvalidFaces(removedEdge.uFirstVertex, removedEdge.uSecondVertex, edges);

			// update edge vector, so any edge that was connected to the removed edge has it's error recalculated
			// and is only connected to the remaining vertex
			for (size_t i = 0; i < edges.size(); i++) {
				if (edges[i].uSecondVertex == removedEdge.uSecondVertex)
					edges[i].uSecondVertex = removedEdge.uFirstVertex;
				else if (edges[i].uFirstVertex == removedEdge.uSecondVertex)
					edges[i].uFirstVertex = removedEdge.uFirstVertex;
			}

			errors[removedEdge.uFirstVertex] = _CalculateVertexErrorQuadric(removedEdge.uFirstVertex);

			for (size_t i = 0; i < edges.size(); i++) {
				if (edges[i].uFirstVertex == removedEdge.uFirstVertex || edges[i].uSecondVertex == removedEdge.uFirstVertex)
					_CalculateEdgeError(edges[i], m_generatedVertices, errors);
			}

			make_heap(edges.begin(), edges.end(), MultiAttributeEdge::CompareEdge());
		}
	}

	vector<uint32_t> MultiAttributeLodGenerator::GetLodIndices() {
		vector<uint32_t> indices;
		indices.reserve(m_generatedIndices.size());

		for (list<uint32_t>::iterator it = m_generatedIndices.begin(); it != m_generatedIndices.end(); it++)
			indices.push_back(*it);

		return indices;
	}
}