﻿#pragma once
#include <vector>
#include <Engine/Math/sVector.h>
#include <Engine/Math/cMatrix_transformation.h>
#include <Engine/Platform/Platform.h>
#include <Engine/Logging/Logging.h>
#include <string>
#include <iostream>
#include <sstream>  
namespace PlutoShe
{
	namespace Physics
	{
		static float eps = 1e-5f;

		class Vector3
		{
		public:
			float m_x, m_y, m_z;

			Vector3() { m_x = m_y = m_z = 0; }
			Vector3(float x, float y, float z) { m_x = x; m_y = y; m_z = z; }
			Vector3(const Vector3& c) { this->Equal(c); }
			Vector3(const eae6320::Math::sVector& c) { m_x = c.x; m_y = c.y; m_z = c.z; }

			void operator = (const Vector3& c) { this->Equal(c); }
			void Equal(const Vector3& c) { m_x = c.m_x; m_y = c.m_y; m_z = c.m_z; }

			Vector3 operator + (const Vector3& i_v) { return Vector3(m_x + i_v.m_x, m_y + i_v.m_y, m_z + i_v.m_z); }
			Vector3 operator - (const Vector3& i_v) { return Vector3(m_x - i_v.m_x, m_y - i_v.m_y, m_z - i_v.m_z); }
			Vector3 operator * (const Vector3& i_v) { return Vector3(m_x * i_v.m_x, m_y * i_v.m_y, m_z * i_v.m_z); }
			Vector3 operator / (const Vector3& i_v) { return Vector3(m_x / i_v.m_x, m_y / i_v.m_y, m_z / i_v.m_z); }

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

			friend Vector3 operator *(eae6320::Math::cMatrix_transformation &i_m, const Vector3 &i_rhs) 
			{
				return i_m * eae6320::Math::sVector(i_rhs.m_x, i_rhs.m_y, i_rhs.m_z);
			}

			void Normalized()
			{
				*this = *this / sqrt(this->dot(*this));
			}

			eae6320::Math::sVector TosVector()
			{
				return eae6320::Math::sVector(m_x, m_y, m_z);
			}
			static void Barycentric(Vector3 p, Vector3 a, Vector3 b, Vector3 c, float& u, float& v, float& w)
			{
				Vector3 v0 = b - a, v1 = c - a, v2 = p - a;
				float d00 = v0.dot(v0);
				float d01 = v0.dot(v1);
				float d11 = v1.dot(v1);
				float d20 = v2.dot(v0);
				float d21 = v2.dot(v1);
				float denom = d00 * d11 - d01 * d01;
				v = (d11 * d20 - d01 * d21) / denom;
				w = (d00 * d21 - d01 * d20) / denom;
				u = 1.0f - v - w;
			}
		};

		


		class simplexPoint  {
		public:
			simplexPoint() 
			{
				m_indexA = m_indexB = 0;
			}
			simplexPoint(Vector3 i_point, int i_a, int i_b)  {
				m_position = i_point;
				m_indexA = i_a;
				m_indexB = i_b;
			}
			simplexPoint(const simplexPoint& i_v)
			{
				m_position = i_v.m_position;
				m_indexA = i_v.m_indexA;
				m_indexB = i_v.m_indexB;
			}
			Vector3 m_position;
			int m_indexA, m_indexB;
			bool operator == (const simplexPoint& i_v)
			{
				return (m_indexA == i_v.m_indexA) && (m_indexB == i_v.m_indexB);
			}
			void operator =(const simplexPoint& i_v)
			{
				m_position = i_v.m_position;
				m_indexA = i_v.m_indexA;
				m_indexB = i_v.m_indexB;
			}
			std::string getKey() 
			{
				std::ostringstream ss;
				ss << m_indexA << ' ' << m_indexB << ' ';
				return ss.str();

			}
		};

		struct SimplexEdge
		{
			simplexPoint a, b;
			SimplexEdge() {}
			SimplexEdge(simplexPoint i_a, simplexPoint i_b) : a(i_a), b(i_b) {}
			bool operator == (const SimplexEdge& i_v)
			{
				return ((a == i_v.a) && (b == i_v.b)) || ((a == i_v.b) && (b == i_v.a));
			}
			std::string GetKey() {
				return a.getKey() + b.getKey();
			}
			std::string getReverseKey()
			{
				return b.getKey() + a.getKey();
			}
		};

		class Face
		{
		public:
			Face() {}
			Face(simplexPoint i_a, simplexPoint i_b, simplexPoint i_c) { a = i_a; b = i_b; c = i_c; }
			simplexPoint a, b, c;
		};

		class Simplex
		{
		public:
			std::vector<simplexPoint> m_points;

			Simplex() { m_points.clear(); }
			size_t GetSize() { return m_points.size(); }
			void Clear() { m_points.clear(); }
			simplexPoint GetA() { return m_points[0]; }
			simplexPoint GetB() { return m_points[1]; }
			simplexPoint GetC() { return m_points[2]; }
			simplexPoint GetD() { return m_points[3]; }
			void RemoveA() { m_points.erase(m_points.begin()); }
			void RemoveB() { m_points.erase(m_points.begin() + 1); }
			void RemoveC() { m_points.erase(m_points.begin() + 2); }
			void RemoveD() { m_points.erase(m_points.begin() + 3); }
			void Add(simplexPoint i_data) { m_points.push_back(i_data); }
			simplexPoint GetLast() { return m_points[m_points.size() - 1]; }
			bool ContainsOrigin(Vector3& i_d);
		};

		class Collider
		{
		public:
			Collider();
			Collider(std::vector<Vector3>& i_v);
			Collider(const Collider& i_v);
			Collider(std::string i_path);

			eae6320::cResult InitData(std::string i_path);;
			void UpdateTransformation(eae6320::Math::cMatrix_transformation i_t);
			Vector3 Center();
			bool IsCollided(Collider& i_B);
			bool IsCollidedReturnSimplex(Collider& i_B, Simplex &t_simplex);
			bool IsCollidedWithContact(Collider& i_B, float& t_depth, Vector3& t_contactNormal, Vector3& t_contactPointA, Vector3& t_contactPointB);
			float GetCollisionContact(Collider& i_B, Simplex& i_simplex, Vector3& t_contactNormal, Vector3& t_contactPointA, Vector3& t_contactPointB);
			
			std::vector<Vector3> m_vertices;
		private:
			//static Vector3 supportFunction(Collider& i_A, Collider& i_B, Vector3 i_dir);
			//static Vector3 supportFunction(Collider& i_A, Collider& i_B, Vector3 i_dir, int& t_indexA, int& t_indexB);
			static simplexPoint supportFunction(Collider& i_A, Collider& i_B, Vector3 i_dir);
			Vector3 getFarthestPointInDirection(Vector3 i_dir, int& t_index);
			Vector3 getFarthestPointInDirection(Vector3 i_dir);
			eae6320::Math::cMatrix_transformation m_transformation;

		};	

		class ColliderList
		{
		public:
			ColliderList();
			ColliderList(const Collider& i_c);
			ColliderList(const ColliderList& i_c);

			void ClearAllCollider();
			void AddCollider(Collider i_c);
			size_t GetSize();
			Collider GetColliderByIndex(int i_index);
			bool IsCollided(ColliderList& i_queryColliderList);
			void UpdateTransformation(eae6320::Math::cMatrix_transformation i_t);

			Vector3 GetCenter();

			std::vector<Collider> m_colliders;
		};

		
	}
}