#pragma once
#include <vector>

namespace PlutoShe
{
	namespace Physics
	{
		class Vector3
		{
		public:
			float m_x, m_y, m_z;

			Vector3() { m_x = m_y = m_z = 0; }
			Vector3(float x, float y, float z) { m_x = x; m_y = y; m_z = z; }
			Vector3(const Vector3& c) { this->Equal(c); }
			void operator = (const Vector3& c) { this->Equal(c); }
			void Equal(const Vector3& c) { m_x = c.m_x; m_y = c.m_y; m_z = c.m_z; }

			Vector3 operator + (const Vector3& i_v) { return Vector3(m_x + i_v.m_x, m_y + i_v.m_y, m_z + i_v.m_z); }
			Vector3 operator - (const Vector3& i_v) { return Vector3(m_x - i_v.m_x, m_y - i_v.m_y, m_z - i_v.m_z); }

			Vector3 operator + (const float i_v) { return Vector3(m_x + i_v, m_y + i_v, m_z + i_v); }
			Vector3 operator - (const float i_v) { return Vector3(m_x - i_v, m_y - i_v, m_z - i_v); }
			Vector3 operator * (const float i_v) { return Vector3(m_x * i_v, m_y * i_v, m_z * i_v); }
			Vector3 operator / (const float i_v) { return Vector3(m_x / i_v, m_y / i_v, m_z / i_v); }

			Vector3 cross(const Vector3& i_v) {
				return Vector3(m_y * i_v.m_z - m_z * i_v.m_y, -(m_x * i_v.m_z - m_z * i_v.m_x), m_x * i_v.m_y - m_y * i_v.m_x);
			}
			Vector3 Negate()
			{
				return *this * -1;
			}
			float dot(const Vector3& i_v)
			{
				return m_x * i_v.m_x + m_y * i_v.m_y + m_z * i_v.m_z;
			}
		};
		class Polythedron
		{
		public:
			Polythedron() { m_vertices.clear(); }
			Polythedron(std::vector<Vector3>& i_v) { m_vertices = i_v; }
			Polythedron(const Polythedron& i_v) { m_vertices = i_v.m_vertices; }
			std::vector<Vector3> m_vertices;
			Vector3 getFarthestPointInDirection(Vector3 i_dir)
			{
				int selection = 0;
				float maxDist = m_vertices[0].dot(i_dir);
				for (size_t i = 1; i < m_vertices.size(); i++)
				{
					float dist = m_vertices[i].dot(i_dir);
					if (dist > maxDist)
					{
						maxDist = dist;
						selection = (int)i;
					}
				} 
				return m_vertices[selection];
			}
			Vector3 Center() {
				Vector3 center;
				int count = 0;
				for (size_t i = 0; i < m_vertices.size(); i++)
				{
					center = center + m_vertices[i];
					count++;
				}
				return center / float(count);
			}
		};
	
		class Simplex
		{
		public:
			std::vector<Vector3> m_points;
			
			Simplex() { m_points.clear(); }
			size_t GetSize() { return m_points.size(); }
			void Clear() { m_points.clear(); }
			Vector3 GetA() { return m_points[0]; }
			Vector3 GetB() { return m_points[1]; }
			Vector3 GetC() { return m_points[2]; }
			Vector3 GetD() { return m_points[3]; }
			void RemoveA() { m_points.erase(m_points.begin()); }
			void RemoveB() { m_points.erase(m_points.begin() + 1); }
			void RemoveC() { m_points.erase(m_points.begin() + 2); }
			void RemoveD() { m_points.erase(m_points.begin() + 3); }
			void Add(Vector3 i_data) { m_points.push_back(i_data); } 
			Vector3 GetLast() { return m_points[m_points.size() - 1]; }
		};

		bool IsCollided(Polythedron &i_A, Polythedron &i_B);

		bool ContainsOrigin(Simplex &i_s, Vector3 &i_d);
		Vector3 SupportFunction(Polythedron& i_A, Polythedron& i_B, Vector3 i_dir);
	}
}