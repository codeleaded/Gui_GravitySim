#include "/home/codeleaded/System/Static/Library/WindowEngine1.0.h"
#include "/home/codeleaded/System/Static/Library/Lib3D_Cube.h"
#include "/home/codeleaded/System/Static/Library/Lib3D_Mathlib.h"
#include "/home/codeleaded/System/Static/Library/Lib3D_Mesh.h"

#define G		1.0f
#define FAKTOR	0.01f

#define FIELDX	40
#define FIELDZ	40


typedef struct Sphere{
	vec3d p;
	vec3d v;
	vec3d a;
	float r;
	float m;
	Pixel c;
} Sphere;

Sphere Sphere_New(vec3d p,vec3d v,vec3d a,float r,float m,Pixel c){
	Sphere s;
	s.p = p;
	s.v = v;
	s.a = a;
	s.r = r;
	s.m = m;
	s.c = c;
	return s;
}
float Sphere_Gravity(Sphere* s,vec3d p){
	vec3d dir = vec3d_Sub(p,s->p);
	float r = vec3d_Length(dir);
	return G * s->m / (r * r);
}
void Sphere_AddGravity(Sphere* s,Sphere* other,float w->ElapsedTime){
	float f = Sphere_Gravity(other,s->p);
	vec3d a = vec3d_Mul(vec3d_Normalise(vec3d_Sub(other->p,s->p)),f);
	
	s->a = vec3d_Add(s->a,vec3d_Mul(a,w->ElapsedTime));
}
char Sphere_isCollision(Sphere* s,Sphere* other){
	float d = vec3d_Length(vec3d_Sub(other->p,s->p));
    return d < (s->r + other->r);
}
void Sphere_Collision(Sphere* s,Sphere* other){
	if(Sphere_isCollision(s,other)){
		vec3d d = vec3d_Sub(s->p,other->p);
		float h = vec3d_Length(d);
		float Overlap = 0.5f * (h - s->r - other->r);
	
		s->p = vec3d_Sub(s->p,vec3d_Div(vec3d_Mul(d,Overlap),h));
		other->p = vec3d_Add(other->p,vec3d_Div(vec3d_Mul(d,Overlap),h));
	}
}
void Sphere_Update(Sphere* s,float w->ElapsedTime){
	s->v = vec3d_Add(s->v,vec3d_Mul(s->a,w->ElapsedTime));
	s->p = vec3d_Add(s->p,vec3d_Mul(s->v,w->ElapsedTime));

	s->a = vec3d_Null();
}
void Sphere_Render(Sphere* s,Vector* tris){
	for(float i = 0.0f;i<2 * F32_PI;i+=0.2f){
		mat4x4 matX = Matrix_MakeRotationX(i);
		mat4x4 matXt = Matrix_MakeRotationX(i+0.2f);

		for(float j = 0.0f;j<2 * F32_PI;j+=0.2f){
			mat4x4 matY = Matrix_MakeRotationY(j);
			mat4x4 matYt = Matrix_MakeRotationY(j+0.2f);

			mat4x4 mat00 = Matrix_MultiplyMatrix(matX,	matY);
			mat4x4 mat10 = Matrix_MultiplyMatrix(matXt,	matY);
			mat4x4 mat01 = Matrix_MultiplyMatrix(matX,	matYt);
			mat4x4 mat11 = Matrix_MultiplyMatrix(matXt,	matYt);
			
			vec3d v00 = Matrix_MultiplyVector(mat00,vec3d_New(0.0f,0.0f,1.0f));
			vec3d v10 = Matrix_MultiplyVector(mat10,vec3d_New(0.0f,0.0f,1.0f));
			vec3d v01 = Matrix_MultiplyVector(mat01,vec3d_New(0.0f,0.0f,1.0f));
			vec3d v11 = Matrix_MultiplyVector(mat11,vec3d_New(0.0f,0.0f,1.0f));

			triangle t1 = { .p = { v00,v10,v11 }, .c = s->c };
			triangle_CalcNorm(&t1);
			triangle_ShadeNorm(&t1,vec3d_New(0.5f,0.6f,0.7f));
			triangle_Scale(&t1,s->r);
			triangle_Offset(&t1,s->p);

			triangle t2 = { .p = { v00,v11,v01 }, .c = s->c };
			triangle_CalcNorm(&t2);
			triangle_ShadeNorm(&t2,vec3d_New(0.5f,0.6f,0.7f));
			triangle_Scale(&t2,s->r);
			triangle_Offset(&t2,s->p);

			Vector_Push(tris,&t1);
			Vector_Push(tris,&t2);
		}
	}
}


Camera cam;
World3D world;
int Mode = 0;
int Menu = 0;
float Speed = 4.0f;

Vector spheres;


void Menu_Set(int m){
	if(Menu==0 && m==1){
		AlxWindow_Mouse_SetInvisible(&window);
		SetMouse((Vec2){ GetWidth() / 2,GetHeight() / 2 });
	}
	if(Menu==1 && m==0){
		AlxWindow_Mouse_SetVisible(&window);
	}
	
	Menu = m;
}

void Setup(AlxWindow* w){
	Menu_Set(1);

	cam = Camera_Make(
		(vec3d){ 0.0f,5.0f,-25.0f,1.0f },
		(vec3d){ 0.0f,0.0f,0.0f,1.0f },
		(vec3d){ 0.0f,0.0f,0.0f,1.0f },
		90.0f
	);

	world = World3D_Make(
		Matrix_MakeWorld((vec3d){ 0.0f,0.0f,0.0f,1.0f },(vec3d){ 0.0f,0.0f,0.0f,1.0f }),
		Matrix_MakePerspektive(cam.p,cam.up,cam.a),
		Matrix_MakeProjection(cam.fov,(float)GetHeight() / (float)GetWidth(),0.1f,1000.0f)
	);
	world.normal = WORLD3D_NORMAL_CAP;

	spheres = Vector_New(sizeof(Sphere));

	Vector_Push(&spheres,(Sphere[]){ Sphere_New(
		vec3d_New(0.0f,0.0f,0.0f),
		vec3d_New(0.0f,0.0f,0.0f),
		vec3d_New(0.0f,0.0f,0.0f),
		1.0f,
		1000000.0f,
		RED
	)});
	Vector_Push(&spheres,(Sphere[]){ Sphere_New(
		vec3d_New(20.0f,0.0f,0.0f),
		vec3d_New(0.0f,0.0f,-17.0f),
		vec3d_New(0.0f,0.0f,0.0f),
		1.0f,
		1000000.0f,
		GREEN
	)});
	Vector_Push(&spheres,(Sphere[]){ Sphere_New(
		vec3d_New(0.0f,0.0f,-30.0f),
		vec3d_New(17.0f,0.0f,0.0f),
		vec3d_New(0.0f,0.0f,0.0f),
		1.0f,
		1000000.0f,
		YELLOW
	)});
	Vector_Push(&spheres,(Sphere[]){ Sphere_New(
		vec3d_New(-40.0f,0.0f,0.0f),
		vec3d_New(0.0f,0.0f,17.0f),
		vec3d_New(0.0f,0.0f,0.0f),
		1.0f,
		1000000.0f,
		LIGHT_BLUE
	)});
	Vector_Push(&spheres,(Sphere[]){ Sphere_New(
		vec3d_New(0.0f,0.0f,50.0f),
		vec3d_New(-17.0f,0.0f,0.0f),
		vec3d_New(0.0f,0.0f,0.0f),
		1.0f,
		1000000.0f,
		BLUE
	)});
}
void Update(AlxWindow* w){
	if(Menu==1){
		Camera_Focus(&cam,GetMouseBefore(),GetMouse(),GetScreenRect().d);
		Camera_Update(&cam);
		SetMouse((Vec2){ GetWidth() / 2,GetHeight() / 2 });
	}
	
	if(Stroke(ALX_KEY_ESC).PRESSED)
		Menu_Set(!Menu);

	if(Stroke(ALX_KEY_Z).PRESSED)
		Mode = Mode < 3 ? Mode+1 : 0;

	if(Stroke(ALX_KEY_W).DOWN)
		cam.p = vec3d_Add(cam.p,vec3d_Mul(cam.ld,Speed * w->ElapsedTime));
	if(Stroke(ALX_KEY_S).DOWN)
		cam.p = vec3d_Sub(cam.p,vec3d_Mul(cam.ld,Speed * w->ElapsedTime));
	if(Stroke(ALX_KEY_A).DOWN)
		cam.p = vec3d_Add(cam.p,vec3d_Mul(cam.sd,Speed * w->ElapsedTime));
	if(Stroke(ALX_KEY_D).DOWN)
		cam.p = vec3d_Sub(cam.p,vec3d_Mul(cam.sd,Speed * w->ElapsedTime));
	if(Stroke(ALX_KEY_R).DOWN)
		cam.p.y += Speed * w->ElapsedTime;
	if(Stroke(ALX_KEY_F).DOWN)
		cam.p.y -= Speed * w->ElapsedTime;


	for(int i = 0;i<spheres.size;i++){
		Sphere* s = (Sphere*)Vector_Get(&spheres,i);
		
		for(int j = 0;j<spheres.size;j++){
			if(i==j) continue;

			Sphere* other = (Sphere*)Vector_Get(&spheres,j);
			Sphere_Collision(s,other);
			Sphere_AddGravity(s,other,w->ElapsedTime);
		}
	}
	for(int i = 0;i<spheres.size;i++){
		Sphere* s = (Sphere*)Vector_Get(&spheres,i);
		Sphere_Update(s,w->ElapsedTime);
	}

	World3D_Set_Model(&world,Matrix_MakeWorld((vec3d){ 0.0f,0.0f,0.0f,1.0f },(vec3d){ 0.0f,0.0f,0.0f,1.0f }));
	World3D_Set_View(&world,Matrix_MakePerspektive(cam.p,cam.up,cam.a));
	World3D_Set_Proj(&world,Matrix_MakeProjection(cam.fov,(float)GetHeight() / (float)GetWidth(),0.1f,1000.0f));
	
	Vector_Clear(&world.trisIn);
	for(int i = 0;i<spheres.size;i++){
		Sphere* s = (Sphere*)Vector_Get(&spheres,i);
		Sphere_Render(s,&world.trisIn);
	}

	for(int i = -FIELDX;i<FIELDX;i++){
		for(int j = -FIELDZ;j<FIELDZ;j++){
			vec3d v00 = vec3d_New(i,	0.0f,j);
			vec3d v10 = vec3d_New(i+1,	0.0f,j);
			vec3d v01 = vec3d_New(i,	0.0f,j+1);
			vec3d v11 = vec3d_New(i+1,	0.0f,j+1);

			for(int k = 0;k<spheres.size;k++){
				Sphere* s = (Sphere*)Vector_Get(&spheres,k);

				v00.y -= FAKTOR * Sphere_Gravity(s,vec3d_New(v00.x,-1.0f,v00.z));
				v10.y -= FAKTOR * Sphere_Gravity(s,vec3d_New(v10.x,-1.0f,v10.z));
				v01.y -= FAKTOR * Sphere_Gravity(s,vec3d_New(v01.x,-1.0f,v01.z));
				v11.y -= FAKTOR * Sphere_Gravity(s,vec3d_New(v11.x,-1.0f,v11.z));
			}

			triangle t1 = { .p = { v00,v11,v10 }, .c = WHITE };
			triangle_CalcNorm(&t1);
			triangle_ShadeNorm(&t1,vec3d_New(0.5f,0.6f,0.7f));

			triangle t2 = { .p = { v00,v01,v11 }, .c = WHITE };
			triangle_CalcNorm(&t2);
			triangle_ShadeNorm(&t2,vec3d_New(0.5f,0.6f,0.7f));

			Vector_Push(&world.trisIn,&t1);
			Vector_Push(&world.trisIn,&t2);
		}
	}

	Clear(LIGHT_BLUE);

	World3D_update(&world,cam.p);

	for(int i = 0;i<world.trisOut.size;i++){
		triangle* t = (triangle*)Vector_Get(&world.trisOut,i);

		if(Mode==0)
			RenderTriangle(((Vec2){ t->p[0].x, t->p[0].y }),((Vec2){ t->p[1].x, t->p[1].y }),((Vec2){ t->p[2].x, t->p[2].y }),t->c);
		if(Mode==1)
			RenderTriangleWire(((Vec2){ t->p[0].x, t->p[0].y }),((Vec2){ t->p[1].x, t->p[1].y }),((Vec2){ t->p[2].x, t->p[2].y }),t->c,1.0f);
		if(Mode==2){
			RenderTriangle(((Vec2){ t->p[0].x, t->p[0].y }),((Vec2){ t->p[1].x, t->p[1].y }),((Vec2){ t->p[2].x, t->p[2].y }),t->c);
			RenderTriangleWire(((Vec2){ t->p[0].x, t->p[0].y }),((Vec2){ t->p[1].x, t->p[1].y }),((Vec2){ t->p[2].x, t->p[2].y }),WHITE,1.0f);
		}
	}

	String str = String_Format("X: %f, Y: %f, Z: %f",cam.p.x,cam.p.y,cam.p.z);
	RenderCStrSize(str.Memory,str.size,0,0,RED);
	String_Free(&str);
	str = String_Format("SizeIn: %d, SizeBuff: %d, SizeOut: %d",world.trisIn.size,world.trisBuff.size,world.trisOut.size);
	RenderCStrSize(str.Memory,str.size,0,window.AlxFont.CharSizeY + 1,RED);
	String_Free(&str);
}
void Delete(AlxWindow* w){
	Vector_Free(&spheres);

	World3D_Free(&world);
	AlxWindow_Mouse_SetVisible(&window);
}

int main(){
    if(Create("Gravity Simulation",2500,1200,1,1,Setup,Update,Delete))
        Start();
    return 0;
}