#pragma once

#ifndef PHYSICS_H
#define PHYSICS_H

#include <DirectXMath.h>

#include "Geometry_Classes.h"
#include "MathFunctions.h"


using namespace DirectX;


/*
DirectX::XMVECTOR Normalize(DirectX::FXMVECTOR v)
{

	float length = XMVectorGetByIndex(XMVector3Length(v), 0);

	if (!length) return XMVectorZero();

	DirectX::XMVECTOR vOut= v;

	return (vOut/length);

}
 */

void CollideClothWithBall(geometry_objects::VisualGeometryObject * const cloth, geometry_objects::VisualGeometryObject * const ball, 
									const bool isApplyGravity, float dt)
{

	float r= ball->GetRadius();
	float distanceToBallCenter = 0;
	float displacement = 0;

	 DirectX::XMVECTOR dir= XMVectorSet(0, 0, 0, 0);


	static DirectX::XMVECTOR Force= XMVectorSet(0, 0, 0, 0);


	DWORD i = 0;


	static float k = 0;



	static float cosine_prev = -1;



	for(i = 0; i < cloth->NumVertices-1; i++)
	{

		dir = ball->Position - cloth->vertices[i].pos;

		distanceToBallCenter = XMVectorGetByIndex(XMVector3Length(dir), 0);

		displacement = distanceToBallCenter-r;

	

	


		if ( displacement < 0 )
		{

			cloth->inCollision= true;
			ball->inCollision= true;
		
			float cosine = XMVectorGetByIndex(XMVector3Dot( XMVector3Normalize( ball->GetForce() ) , XMVector3Normalize(-dir)), 0);


			if (cosine != cosine_prev && abs(acos(cosine)-acos(cosine_prev)) > 5*Pi/180)
			{

				ball->inCollision = true;

				
				ball->counter++;

				
				ball->angles.clear();
				ball->angles.push_back(acos(cosine)*180/Pi);

				
				float sine = sin(acos(cosine));

				
						DirectX::XMVECTOR Force_cloth = ball->GetForce();
						DirectX::XMVECTOR Force_ball  = -Force_cloth+Force*0;


					ball->F_ball.push_back(Force_ball);
					ball->F_cloth.push_back(Force_cloth);
			

				DirectX::XMVECTOR Velocity_cloth = (Force_cloth/ball->m)*dt*2.5; // скорость на выходе 
														//меньше, чем скорость на входе за счЄт угла взаимодействи€ и коэффициента диссипации


					float dV = XMVectorGetByIndex( XMVector3Length((ball->GetVelocity() - ball->velocity_prev)), 0 );


				if (cloth->Kv < 1.0001) cloth->Kv = 1-dV*1e1*1.15;
				 else cloth->Kv = 1;

				  cloth->vertices[i].vel += Velocity_cloth;
				  ball->AddForce(Force_ball);
				  //ball->AddForce(Force_ball*cloth->Kv);
				  //ball->AddForce(Force_ball*k);

				 if( k < 1.0001 ) k += 1e-2;

				 ball->Kv_coefs.push_back(k);

				cosine_prev = cosine;

			}
			

		}


	}


	//cloth->ApplyForce(dt, isApplyGravity); // в ApplyForce() и в SimulateClothDynamics() используетс€ один и тот же алгоритм применени€ силы
	dynamic_cast<geometry_objects::Cloth*>(cloth)->SimulateClothDynamics(dt, Force); // => пользуемс€ только SimulateClothDynamics()
	dynamic_cast<geometry_objects::Sphere*>(ball)->ApplyForce(dt, isApplyGravity);
	

}






void Collide(geometry_objects::VisualGeometryObject * const obj_a, geometry_objects::VisualGeometryObject * const obj_b, float dt)
{

	DirectX::XMVECTOR centersLineDir= XMVectorSet(0, 0, 0, 0),
				aDirPoint= XMVectorSet(0, 0, 0, 0), bDirPoint= XMVectorSet(0, 0, 0, 0);

	float dist= 1e3; // рассто€ние между сферами в направлении их сближени€


	centersLineDir= XMVector3Normalize(obj_b->Position-obj_a->Position);

	aDirPoint= obj_a->Position + obj_a->r*centersLineDir;
	bDirPoint= obj_b->Position - obj_b->r*centersLineDir;

	dist = XMVectorGetByIndex(XMVector3Length( (bDirPoint - aDirPoint) ), 0);
	 
		
/*
 //if (dist < 0)
	if = XMVectorSet(0 == dist)
 {
 */



		DWORD i0, i1, i2, j0, j1, j2;
		DirectX::XMVECTOR distV_i0j0, distV_i1j1, distV_i2j2, dist_avgV, dist_avgDir;
		DirectX::XMVECTOR normal_a= XMVectorSet(0, 0, 0, 0), normal_b= XMVectorSet(0, 0, 0, 0);
		DirectX::XMVECTOR Force= XMVectorSet(0, 0, 0, 0);
		DirectX::XMVECTOR dir= XMVectorSet(0, 0, 0, 0), dir_avg_i= XMVectorSet(0, 0, 0, 0), dir_avgTotal= XMVectorSet(0, 0, 0, 0);
		float dist_avg;
		float cosine = 0, cosine_avg = 0, cosine_avg_i = 0, cosine_avgTotal = 0;

		float u= 0, t = 0;

		float numberOfInteractions = 0;
		DirectX::XMVECTOR avgVelocity= XMVectorSet(0, 0, 0, 0);
		
		


	for (DWORD i = 0; i < obj_a->NumFaces; i++) // проверка каждого треугольника obj_a на столкновение
	{

		
		i0= obj_a->indices[i*3];
		i1= obj_a->indices[i*3+1];
		i2= obj_a->indices[i*3+2];
	
					

			for(DWORD j = 0; j < obj_b->NumFaces; j++) // с каждым треугольником obj_b
			{


					j0= obj_b->indices[j*3];
					j1= obj_b->indices[j*3+1];
					j2= obj_b->indices[j*3+2];
					
							
						// рассто€ние от центра треугольника obj до центра сферы

					distV_i0j0= obj_b->vertices[j0].pos - obj_a->vertices[i0].pos;
				  distV_i1j1= obj_b->vertices[j1].pos - obj_a->vertices[i1].pos;
				distV_i2j2= obj_b->vertices[j2].pos - obj_a->vertices[i2].pos;

				dist_avgV= (distV_i0j0 + distV_i1j1 + distV_i2j2)/3; 
					dist_avg = XMVectorGetByIndex(XMVector3Length(dist_avgV), 0);

					dist_avgDir = XMVector3Normalize(dist_avgV); // направление воздействи€ obj_a на obj_b



					if ( dist_avg  < 0.09 ) // произошло столкновение треугольник-треугольник
					{
				
						dist_avg= dist_avg;

						if (obj_a->Deformable && ! obj_b->Deformable)
						{

						  normal_a = XMVector3Normalize( obj_a->vertices[i0].normal + obj_a->vertices[i1].normal + obj_a->vertices[i2].normal );

						  cosine = XMVectorGetByIndex(XMVector3Dot(dist_avgDir, normal_a), 0);

						   Force = (obj_b->m * obj_b->GetAcceleration())*cosine;


						   // »зменить скорость одинаково дл€ всего треугольника
						   obj_a->vertices[i0].vel= ( -Force / obj_b->m )*dt;
						   obj_a->vertices[i1].vel= obj_a->vertices[i0].vel;
						   obj_a->vertices[i2].vel= obj_a->vertices[i0].vel;


						   
						
						   // ƒл€ obj_b: суммарный вектор обратного воздействи€; 
						   // суммарный коэффициент изменени€ интенсивности воздействи€ы 
						   dir += dist_avgDir;

						   cosine_avg += cosine;

						   avgVelocity += obj_a->vertices[i0].vel;

							numberOfInteractions++;
					    }						

					

					} // if ( dist_avg  < 0 )

					
					
				}// j


				if (obj_a->Deformable && ! obj_b->Deformable && numberOfInteractions)
				{	

					cosine_avg_i += cosine_avg/numberOfInteractions;

					dir_avg_i += dir;

					t = i;

				}

					
				

			
			}// i


				if (obj_a->Deformable && ! obj_b->Deformable && t)
				{


						dir_avgTotal = XMVector3Normalize(dir_avg_i);
					cosine_avgTotal = cosine_avg_i/t;

					//Force = obj_a->m * avgVelocity * dt;


					obj_b->AddVelocity(-avgVelocity*cosine_avgTotal/numberOfInteractions, dt);

					//Force = numberOfInteractions * obj_a->m * avgVelocity / numberOfInteractions;

				}




		
		
	//	}// if (dist < 0)


}


#endif

