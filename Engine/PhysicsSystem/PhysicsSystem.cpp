// PhysicsSystem.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "PhysicsSystem.h"
#include <map>

namespace PlutoShe
{
	namespace Physics
	{

		Vector3 Collider::getFarthestPointInDirection(Vector3 i_dir, int &t_index)
		{
			t_index = 0;
			float maxDist = (m_transformation * m_vertices[0]).dot(i_dir);
			for (size_t i = 1; i < m_vertices.size(); i++)
			{
				float dist = (m_transformation * m_vertices[i]).dot(i_dir);
				if (dist > maxDist)
				{
					maxDist = dist;
					t_index = (int)i;
				}
			}
			return m_transformation * m_vertices[t_index];
		}

		Vector3 Collider::getFarthestPointInDirection(Vector3 i_dir)
		{
			int index;
			return getFarthestPointInDirection(i_dir, index);
		}

		simplexPoint Collider::supportFunction(Collider& i_A, Collider& i_B, Vector3 i_dir)
		{
			int indexA, indexB;
			auto a = i_A.getFarthestPointInDirection(i_dir, indexA);
			auto b = i_B.getFarthestPointInDirection(i_dir.Negate(), indexB);
			return simplexPoint(a - b, indexA, indexB);
		}


		Vector3 Collider::Center()
		{
			Vector3 center;
			int count = 0;
			for (size_t i = 0; i < m_vertices.size(); i++)
			{
				center = center + m_transformation * m_vertices[i];
				count++;
			}
			return center / float(count);
		}

		/*float Collider::GetClosetFaceDistance( &t_dir)
		{

			if (GetSize() < 4) return -1;
			float minDist = this->GetD().dot(this->GetD());
			int index = 0;
			for (int i = 0; i < 4; i++)
			{
				auto a = m_points[(i + 1) % 4];
				auto b = m_points[(i + 2) % 4];
				auto c = m_points[(i + 3) % 4];
				auto ab = b - a;
				auto ac = c - a;
				auto normal_abc = ab.cross(ac);
				auto ndac = normal_abc.dot(a);
				if (ndac < minDist)
				{
					minDist = ndac;
					t_dir = normal_abc;
					index = i;
				}
			}
			m_points.erase(m_points.begin() + index);
			return minDist;
		}*/

		float Collider::GetCollisionContact(Collider& i_B, Simplex& i_simplex, Vector3& t_contactNormal, Vector3& t_contactPointA, Vector3& t_contactPointB)
		{
			simplexPoint a, b, c, d;
			a = i_simplex.GetA();
			b = i_simplex.GetB();
			c = i_simplex.GetC();
			d = i_simplex.GetD();
			std::vector<Face> faces;
			faces.push_back(Face(a, c, b));
			faces.push_back(Face(d, a, b));
			faces.push_back(Face(d, b, c));
			faces.push_back(Face(d, c, a));

			while (true)
			{
				float minDist = faces[0].a.m_position.dot(faces[0].a.m_position);
				Vector3 selectedFaceNormal;
				size_t selectedFaceIndex = 0;
				for (size_t i = 0; i < faces.size(); i++)
				{
					auto a = faces[i].a;
					auto b = faces[i].b;
					auto c = faces[i].c;
					auto ab = b.m_position - a.m_position;
					auto ac = c.m_position - a.m_position;
					auto normal_abc = ab.cross(ac);
					normal_abc.Normalized();
					auto nabc = normal_abc.dot(a.m_position);
					if (nabc < minDist)
					{
						minDist = nabc;
						selectedFaceNormal = normal_abc;
						selectedFaceIndex = i;
					}
				}
				
				//if (minDist > lastMinDist)
				//{
				//	return lastMinDist;
				//}


				auto nextPoint = supportFunction(*this, i_B, selectedFaceNormal);
				double d = nextPoint.m_position.dot(selectedFaceNormal);

				if (d - minDist < eps)
				{
					selectedFaceNormal.Normalized();
					t_contactNormal = selectedFaceNormal;

					auto a = faces[selectedFaceIndex].a;
					auto b = faces[selectedFaceIndex].b;
					auto c = faces[selectedFaceIndex].c;
					auto a1 = m_vertices[faces[selectedFaceIndex].a.m_indexA];
					auto a2 = m_vertices[faces[selectedFaceIndex].b.m_indexA];
					auto a3 = m_vertices[faces[selectedFaceIndex].c.m_indexA];
					auto b1 = i_B.m_vertices[faces[selectedFaceIndex].a.m_indexB];
					auto b2 = i_B.m_vertices[faces[selectedFaceIndex].b.m_indexB];
					auto b3 = i_B.m_vertices[faces[selectedFaceIndex].c.m_indexB];
					//auto a3 = m_vertices[faces[selectedFaceIndex].c.m_indexA];
					float u, v, w;
					Vector3::Barycentric(Vector3(0, 0, 0), a.m_position, b.m_position, c.m_position, u, v, w);

					t_contactPointA = a1 * u + a2 * v + a3 * w;
					t_contactPointB = b1 * u + b2 * v + b3 * w;

					return d;
				}
				else
				{
					for (int i = faces.size() - 1; i >= 0; i--)
					{
						auto a = faces[i].a;
						auto b = faces[i].b;
						auto c = faces[i].c;
						auto ab = b.m_position - a.m_position;
						auto ac = c.m_position - a.m_position;
						auto normal_abc = ab.cross(ac);
						normal_abc.Normalized();
						if (normal_abc.dot(nextPoint.m_position - a.m_position) > -eps)
						{
							faces.erase(faces.begin() + i);
						}
					}
					std::map<std::string, SimplexEdge> edges;
					for (int i = 0; i < faces.size(); i++)
					{
						auto a = faces[i].a;
						auto b = faces[i].b;
						auto c = faces[i].c;
						auto ab = SimplexEdge(a, b);
						auto ca = SimplexEdge(c, a);
						auto bc = SimplexEdge(b, c);
						if (edges.find(ab.getReverseKey()) != edges.end()) {
							edges.erase(ab.getReverseKey());
						}
						else {
							edges.insert(std::pair<std::string, SimplexEdge>(ab.GetKey(), ab));
						}
						if (edges.find(ca.getReverseKey()) != edges.end()) {
							edges.erase(ca.getReverseKey());
						}
						else {
							edges.insert(std::pair<std::string, SimplexEdge>(ca.GetKey(), ca));
						}
						if (edges.find(bc.getReverseKey()) != edges.end()) {
							edges.erase(bc.getReverseKey());
						}
						else {
							edges.insert(std::pair<std::string, SimplexEdge>(bc.GetKey(), bc));
						}
					}
					for (auto it = edges.begin(); it != edges.end(); it++)
					{

						faces.push_back(Face(nextPoint, it->second.b, it->second.a));

					}
				}


			}
		}

		bool Collider::IsCollidedReturnSimplex(Collider& i_B, Simplex &t_simplex)
		{
			bool isCollided = false;
			//GJK
			Vector3 dir = i_B.m_transformation * i_B.Center() - this->m_transformation * this->Center();
			dir = dir + 0.1f;
			t_simplex.Clear();
			while (true)
			{
				t_simplex.Add(supportFunction(*this, i_B, dir));
				auto dist = t_simplex.GetLast().m_position.dot(dir);
				if (dist < -eps) {
					return false;
				}
				else {
					if (t_simplex.ContainsOrigin(dir)) {
						return true;
					}
				}
			}

		}

		bool Collider::IsCollided(Collider& i_B)
		{
			Simplex simplex;
			return IsCollidedReturnSimplex(i_B, simplex);
		}

		bool Collider::IsCollidedWithContact(Collider&i_B, float &t_depth, Vector3 &t_contactNormal, Vector3& t_contactPointA, Vector3& t_contactPointB)
		{
			Simplex simplex;
			if (IsCollidedReturnSimplex(i_B, simplex))
			{
				t_depth = GetCollisionContact(i_B, simplex, t_contactNormal, t_contactPointA, t_contactPointB);
				return true;
			}
			return false;
		}

		bool Simplex::ContainsOrigin(Vector3 &t_direction)
		{
			simplexPoint a, b, c, d;
			Vector3 ab, ac, ao;
			switch (this->GetSize())
			{
			case 1:
				t_direction = t_direction * -1;
				break;
			case 2:
				a = this->GetA();
			    b = this->GetB();
				ab = b.m_position - a.m_position;
				ao = a.m_position.Negate();
				t_direction = ab.cross(ao).cross(ab);
				if (t_direction.m_x == 0 && t_direction.m_y == 0 && t_direction.m_z == 0) {
					t_direction.m_x = ao.m_y;
					t_direction.m_y = ao.m_x;
				}
				break;

			case 3:
				a = this->GetA();
				b = this->GetB();
				c = this->GetC();
				ab = b.m_position - a.m_position;
				ac = c.m_position - a.m_position;
				ao = a.m_position.Negate();
				t_direction = ab.cross(ac);
				if (t_direction.dot(ao) < 0) {
					t_direction = t_direction * -1;
					auto tmp = this->m_points[1];
					this->m_points[2] = this->m_points[1];
					this->m_points[1] = tmp;
				}
				break;
			case 4:
			    a = this->GetA();
				b = this->GetB();
				c = this->GetC();
				d = this->GetD();
				auto da = a.m_position - d.m_position;
				auto db = b.m_position - d.m_position;
				auto dc = c.m_position - d.m_position;
				auto normal_dab = da.cross(db);
				auto normal_dac = dc.cross(da);
				auto normal_dbc = db.cross(dc);
				auto do_ = d.m_position.Negate(); // (0,0,0) - point d
				auto ndab = normal_dab.dot(do_); 
				auto ndac = normal_dac.dot(do_);
				auto ndbc = normal_dbc.dot(do_);
				if (ndab > 0)
				{
					this->RemoveC();

					t_direction = normal_dab;
				}
				else if (ndac > 0)
				{
					this->m_points[1] = d;
					this->RemoveD();
					t_direction = normal_dac;
				}
				else if (ndbc > 0)
				{
					this->m_points[0] = d;
					this->RemoveD();
					t_direction = normal_dbc;
				}
				else
				{
					return true;
				}
				break;
			}
			return false;
		}

		eae6320::cResult PlutoShe::Physics::Collider::InitData(std::string i_path)
		{
			auto result = eae6320::Results::Success;
			std::string errorMessage;
			eae6320::Platform::sDataFromFile dataFromFile;

			auto resultReadBinaryFile = eae6320::Platform::LoadBinaryFile(i_path.c_str(), dataFromFile, &errorMessage);
			if (!resultReadBinaryFile)
			{
				result = resultReadBinaryFile;
				eae6320::Logging::OutputError("Couldn't read binary file at path %s", i_path.c_str());
				return result;
			}

			auto currentOffset = reinterpret_cast<uintptr_t>(dataFromFile.data);
			uint16_t vertexCount;
			size_t sizeS = 0;
			if (sizeS + sizeof(uint16_t) > dataFromFile.size)
			{
				eae6320::Logging::OutputError("Wrong file size at path %s", i_path.c_str());
				return result;
			}

			memcpy(&vertexCount, reinterpret_cast<void*>(currentOffset), sizeof(uint16_t));
			
			currentOffset += sizeof(uint16_t);
			sizeS += sizeof(uint16_t);
			if (sizeS + vertexCount * sizeof(Vector3) > dataFromFile.size)
			{
				eae6320::Logging::OutputError("Wrong file size at path %s", i_path.c_str());
				return result;
			}

			m_vertices.resize(vertexCount);
			memcpy(&m_vertices[0], reinterpret_cast<void*>(currentOffset), vertexCount * sizeof(Vector3));
			return result;
		}

		PlutoShe::Physics::Collider::Collider() { m_vertices.clear(); }
		PlutoShe::Physics::Collider::Collider(std::vector<Vector3>& i_v) { m_vertices = i_v; }
		PlutoShe::Physics::Collider::Collider(const Collider& i_v) { m_vertices = i_v.m_vertices; m_transformation = i_v.m_transformation; }
		PlutoShe::Physics::Collider::Collider(std::string i_path) { InitData(i_path); }

		void PlutoShe::Physics::Collider::UpdateTransformation(eae6320::Math::cMatrix_transformation i_t)
		{
			m_transformation = i_t;
		}

		PlutoShe::Physics::ColliderList::ColliderList() { m_colliders.clear(); }
		PlutoShe::Physics::ColliderList::ColliderList(const Collider& i_c) { m_colliders.clear(); m_colliders.push_back(i_c); }
		PlutoShe::Physics::ColliderList::ColliderList(const ColliderList& i_c) { m_colliders = i_c.m_colliders; }


		void PlutoShe::Physics::ColliderList::UpdateTransformation(eae6320::Math::cMatrix_transformation i_t)
		{
			for (int i = 0; i < m_colliders.size(); i++)
			{
				m_colliders[i].UpdateTransformation(i_t);
			}
		}


		bool PlutoShe::Physics::ColliderList::IsCollided(ColliderList& i_queryColliderList)
		{
			for (auto i = 0; i < m_colliders.size(); i++)
			{
				for (auto j = 0; j < i_queryColliderList.m_colliders.size(); j++)
				{
					Vector3 contacts;
					if (m_colliders[i].IsCollided(i_queryColliderList.m_colliders[j]))
					{
						return true;
					}
				}
			}
			return false;
		}


		Collider PlutoShe::Physics::ColliderList::GetColliderByIndex(int i_index)
		{ 
			return m_colliders[i_index]; 
		}

		size_t PlutoShe::Physics::ColliderList::GetSize() { return m_colliders.size(); }

		void PlutoShe::Physics::ColliderList::AddCollider(Collider i_c) { m_colliders.push_back(i_c); }

		void PlutoShe::Physics::ColliderList::ClearAllCollider() { m_colliders.clear(); }

		Vector3 PlutoShe::Physics::ColliderList::GetCenter()
		{
			Vector3 center;
			for (auto i = 0; i < m_colliders.size(); i++)
			{
				center = center + m_colliders[i].Center();
			}
			center = center / m_colliders.size();
			return center;
		}

	}
}
