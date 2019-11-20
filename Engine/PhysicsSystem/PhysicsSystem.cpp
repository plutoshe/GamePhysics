// PhysicsSystem.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "PhysicsSystem.h"

namespace PlutoShe
{
	namespace Physics
	{
		Vector3 Collider::getFarthestPointInDirection(Vector3 i_dir)
		{
			int selection = 0;
			float maxDist = (m_transformation * m_vertices[0]).dot(i_dir);
			for (size_t i = 1; i < m_vertices.size(); i++)
			{
				float dist = (m_transformation * m_vertices[i]).dot(i_dir);
				if (dist > maxDist)
				{
					maxDist = dist;
					selection = (int)i;
				}
			}
			return m_transformation * m_vertices[selection];
		}

		Vector3 Collider::supportFunction(Collider&i_A, Collider&i_B, Vector3 i_dir)
		{
			auto a = i_A.getFarthestPointInDirection(i_dir);
			auto b = i_B.getFarthestPointInDirection(i_dir.Negate());
			return a - b;	
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

		bool Collider::IsCollided(Collider&i_B)
		{
			Vector3 dir = i_B.Center() - this->Center();
			Simplex simplex;
			simplex.Clear();
			while (true)
			{
				simplex.Add(supportFunction(*this, i_B, dir));

				if (simplex.GetLast().dot(dir) < 0) {
					return false;
				}
				else {
					if (simplex.ContainsOrigin(dir)) {
						return true;
					}
				}
			}
		}

		bool Simplex::ContainsOrigin(Vector3 &t_direction)
		{
			Vector3 a, b, c, d;
			Vector3 ab, ac, ao;
			switch (this->GetSize())
			{
			case 1:
				t_direction = t_direction * -1;
				break;
			case 2:
				a = this->GetA();
			    b = this->GetB();
				ab = b - a;
				ao = a.Negate();
				t_direction = ab.cross(ao).cross(ab);
				break;

			case 3:
				a = this->GetA();
				b = this->GetB();
				c = this->GetC();
				ab = b - a;
				ac = c - a;
				ao = a.Negate();
				t_direction = ac.cross(ab);
				if (t_direction.dot(ao) < 0) t_direction = t_direction * -1;
				break;
			case 4:
			    a = this->GetA();
				b = this->GetB();
				c = this->GetC();
				d = this->GetD();
				auto da = a - d;
				auto db = b - d;
				auto dc = c - d;
				auto normal_dab = da.cross(db);
				auto normal_dac = dc.cross(da);
				auto normal_dbc = db.cross(dc);
				auto do_ = d.Negate(); // (0,0,0) - point d
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
	}
}
