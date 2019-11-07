// PhysicsSystem.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "PhysicsSystem.h"

namespace PlutoShe
{
	namespace Physics
	{
		Vector3 SupportFunction(Polythedron &i_A, Polythedron &i_B, Vector3 i_dir)
		{
			auto a = i_A.getFarthestPointInDirection(i_dir);
			auto b = i_B.getFarthestPointInDirection(i_dir);
			return a - b;
		}
			
		bool IsCollided(Polythedron &i_A, Polythedron &i_B)
		{
			Vector3 dir = i_B.Center() - i_A.Center();
			Simplex simplex;
			simplex.Clear();
			while (true)
			{
				simplex.Add(SupportFunction(i_A, i_B, dir));

				if (simplex.GetLast().dot(dir) < 0) {
					return false;
				}
				else {
					if (ContainsOrigin(simplex, dir)) {
						return true;
					}
				}
			}
		}

		bool ContainsOrigin(Simplex &t_s, Vector3 &t_direction)
		{
			Vector3 a, b, c, d;
			Vector3 ab, ac, ao;
			switch (t_s.GetSize())
			{
			case 1:
				t_direction = t_direction * -1;
				break;
			case 2:
				a = t_s.GetA();
			    b = t_s.GetB();
				ab = b - a;
				ao = a.Negate();
				t_direction = ab.cross(ao).cross(ab);
				break;

			case 3:
				a = t_s.GetA();
				b = t_s.GetB();
				c = t_s.GetC();
				ab = b - a;
				ac = c - a;
				ao = a.Negate();
				t_direction = ac.cross(ab);
				if (t_direction.dot(ao) < 0) t_direction = t_direction * -1;
				break;
			case 4:
			    a = t_s.GetA();
				b = t_s.GetB();
				c = t_s.GetC();
				d = t_s.GetD();
				auto da = a - d;
				auto db = b - d;
				auto dc = c - d;
				auto normal_dab = da.cross(db);
				auto normal_dac = da.cross(dc);
				auto normal_dbc = db.cross(dc);
				auto do_ = d.Negate(); // (0,0,0) - point d
				if (normal_dab.dot(do_) > 0)
				{
					t_s.RemoveC();
					t_direction = normal_dab;
				}
				else if (normal_dac.dot(do_) > 0)
				{
					t_s.RemoveB();
					t_direction = normal_dac;
				}
				else if (normal_dbc.dot(do_) > 0)
				{
					t_s.RemoveA();
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
