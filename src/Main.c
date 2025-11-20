#include "/home/codeleaded/System/Static/Library/WindowEngine1.0.h"
#include "/home/codeleaded/System/Static/Library/Lib3D_Cube.h"
#include "/home/codeleaded/System/Static/Library/Lib3D_Mathlib.h"
#include "/home/codeleaded/System/Static/Library/Lib3D_Mesh.h"

#define G		1.0f
#define FAKTOR	0.01f

#define FIELDX	40
#define FIELDZ	40


typedef struct Sphere{
	Vec3D p;
	Vec3D v;
	Vec3D a;
	float r;
	float m;
	Pixel c;
} Sphere;

Sphere Sphere_New(Vec3D p,Vec3D v,Vec3D a,float r,float m,Pixel c){
	Sphere s;
	s.p = p;
	s.v = v;
	s.a = a;
	s.r = r;
	s.m = m;
	s.c = c;
	return s;
}
float Sphere_Gravity(Sphere* s,Vec3D p){
	Vec3D dir = Vec3D_Sub(p,s->p);
	float r = Vec3D_Length(dir);
	return G * s->m / (r * r);
}
void Sphere_AddGravity(Sphere* s,Sphere* other,float ElapsedTime){
	float f = Sphere_Gravity(other,s->p);
	Vec3D a = Vec3D_Mul(Vec3D_Normalise(Vec3D_Sub(other->p,s->p)),f);
	
	s->a = Vec3D_Add(s->a,Vec3D_Mul(a,ElapsedTime));
}
char Sphere_isCollision(Sphere* s,Sphere* other){
	float d = Vec3D_Length(Vec3D_Sub(other->p,s->p));
    return d < (s->r + other->r);
}
void Sphere_Collision(Sphere* s,Sphere* other){
	if(Sphere_isCollision(s,other)){
		Vec3D d = Vec3D_Sub(s->p,other->p);
		float h = Vec3D_Length(d);
		float Overlap = 0.5f * (h - s->r - other->r);
	
		s->p = Vec3D_Sub(s->p,Vec3D_Div(Vec3D_Mul(d,Overlap),h));
		other->p = Vec3D_Add(other->p,Vec3D_Div(Vec3D_Mul(d,Overlap),h));
	}
}
void Sphere_Update(Sphere* s,float ElapsedTime){
	s->v = Vec3D_Add(s->v,Vec3D_Mul(s->a,ElapsedTime));
	s->p = Vec3D_Add(s->p,Vec3D_Mul(s->v,ElapsedTime));

	s->a = Vec3D_Null();
}
void Sphere_Render(Sphere* s,Vector* tris){
	for(float i = 0.0f;i<2 * F32_PI;i+=0.2f){
		M4x4D matX = Matrix_MakeRotationX(i);
		M4x4D matXt = Matrix_MakeRotationX(i+0.2f);

		for(float j = 0.0f;j<2 * F32_PI;j+=0.2f){
			M4x4D matY = Matrix_MakeRotationY(j);
			M4x4D matYt = Matrix_MakeRotationY(j+0.2f);

			M4x4D mat00 = Matrix_MultiplyMatrix(matX,	matY);
			M4x4D mat10 = Matrix_MultiplyMatrix(matXt,	matY);
			M4x4D mat01 = Matrix_MultiplyMatrix(matX,	matYt);
			M4x4D mat11 = Matrix_MultiplyMatrix(matXt,	matYt);
			
			Vec3D v00 = Matrix_MultiplyVector(mat00,Vec3D_New(0.0f,0.0f,1.0f));
			Vec3D v10 = Matrix_MultiplyVector(mat10,Vec3D_New(0.0f,0.0f,1.0f));
			Vec3D v01 = Matrix_MultiplyVector(mat01,Vec3D_New(0.0f,0.0f,1.0f));
			Vec3D v11 = Matrix_MultiplyVector(mat11,Vec3D_New(0.0f,0.0f,1.0f));

			Tri3D t1 = { .p = { v00,v10,v11 }, .c = s->c };
			Tri3D_CalcNorm(&t1);
			Tri3D_ShadeNorm(&t1,Vec3D_New(0.5f,0.6f,0.7f));
			Tri3D_Scale(&t1,s->r);
			Tri3D_Offset(&t1,s->p);

			Tri3D t2 = { .p = { v00,v11,v01 }, .c = s->c };
			Tri3D_CalcNorm(&t2);
			Tri3D_ShadeNorm(&t2,Vec3D_New(0.5f,0.6f,0.7f));
			Tri3D_Scale(&t2,s->r);
			Tri3D_Offset(&t2,s->p);

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
		(Vec3D){ 0.0f,5.0f,-25.0f,1.0f },
		(Vec3D){ 0.0f,0.0f,0.0f,1.0f },
		(Vec3D){ 0.0f,0.0f,0.0f,1.0f },
		(Vec3D){ 0.0f,0.0f,0.0f,1.0f },
		90.0f
	);

	world = World3D_Make(
		Matrix_MakeWorld((Vec3D){ 0.0f,0.0f,0.0f,1.0f },(Vec3D){ 0.0f,0.0f,0.0f,1.0f }),
		Matrix_MakePerspektive(cam.p,cam.up,cam.a),
		Matrix_MakeProjection(cam.fov,(float)GetHeight() / (float)GetWidth(),0.1f,1000.0f)
	);
	world.normal = WORLD3D_NORMAL_CAP;

	spheres = Vector_New(sizeof(Sphere));

	Vector_Push(&spheres,(Sphere[]){ Sphere_New(
		Vec3D_New(0.0f,0.0f,0.0f),
		Vec3D_New(0.0f,0.0f,0.0f),
		Vec3D_New(0.0f,0.0f,0.0f),
		1.0f,
		1000000.0f,
		RED
	)});
	Vector_Push(&spheres,(Sphere[]){ Sphere_New(
		Vec3D_New(20.0f,0.0f,0.0f),
		Vec3D_New(0.0f,0.0f,-17.0f),
		Vec3D_New(0.0f,0.0f,0.0f),
		1.0f,
		1000.0f,
		GREEN
	)});
	Vector_Push(&spheres,(Sphere[]){ Sphere_New(
		Vec3D_New(0.0f,0.0f,-30.0f),
		Vec3D_New(17.0f,0.0f,0.0f),
		Vec3D_New(0.0f,0.0f,0.0f),
		1.0f,
		1000.0f,
		YELLOW
	)});
	Vector_Push(&spheres,(Sphere[]){ Sphere_New(
		Vec3D_New(-40.0f,0.0f,0.0f),
		Vec3D_New(0.0f,0.0f,17.0f),
		Vec3D_New(0.0f,0.0f,0.0f),
		1.0f,
		1000.0f,
		LIGHT_BLUE
	)});
	Vector_Push(&spheres,(Sphere[]){ Sphere_New(
		Vec3D_New(0.0f,0.0f,50.0f),
		Vec3D_New(-17.0f,0.0f,0.0f),
		Vec3D_New(0.0f,0.0f,0.0f),
		1.0f,
		1000.0f,
		BLUE
	)});
}
void Update(AlxWindow* w){
	if(Menu==1){
		Camera_Focus(&cam,GetMouseBefore(),GetMouse(),GetScreenRect().d);
		Camera_Update(&cam,w->ElapsedTime);
		SetMouse((Vec2){ GetWidth() / 2,GetHeight() / 2 });
	}
	
	if(Stroke(ALX_KEY_ESC).PRESSED)
		Menu_Set(!Menu);

	if(Stroke(ALX_KEY_Z).PRESSED)
		Mode = Mode < 3 ? Mode+1 : 0;

	if(Stroke(ALX_KEY_W).DOWN)
		cam.p = Vec3D_Add(cam.p,Vec3D_Mul(cam.ld,Speed * w->ElapsedTime));
	if(Stroke(ALX_KEY_S).DOWN)
		cam.p = Vec3D_Sub(cam.p,Vec3D_Mul(cam.ld,Speed * w->ElapsedTime));
	if(Stroke(ALX_KEY_A).DOWN)
		cam.p = Vec3D_Add(cam.p,Vec3D_Mul(cam.sd,Speed * w->ElapsedTime));
	if(Stroke(ALX_KEY_D).DOWN)
		cam.p = Vec3D_Sub(cam.p,Vec3D_Mul(cam.sd,Speed * w->ElapsedTime));
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

	World3D_Set_Model(&world,Matrix_MakeWorld((Vec3D){ 0.0f,0.0f,0.0f,1.0f },(Vec3D){ 0.0f,0.0f,0.0f,1.0f }));
	World3D_Set_View(&world,Matrix_MakePerspektive(cam.p,cam.up,cam.a));
	World3D_Set_Proj(&world,Matrix_MakeProjection(cam.fov,(float)GetHeight() / (float)GetWidth(),0.1f,1000.0f));
	
	Vector_Clear(&world.trisIn);
	for(int i = 0;i<spheres.size;i++){
		Sphere* s = (Sphere*)Vector_Get(&spheres,i);
		Sphere_Render(s,&world.trisIn);
	}

	for(int i = -FIELDX;i<FIELDX;i++){
		for(int j = -FIELDZ;j<FIELDZ;j++){
			Vec3D v00 = Vec3D_New(i,	0.0f,j);
			Vec3D v10 = Vec3D_New(i+1,	0.0f,j);
			Vec3D v01 = Vec3D_New(i,	0.0f,j+1);
			Vec3D v11 = Vec3D_New(i+1,	0.0f,j+1);

			for(int k = 0;k<spheres.size;k++){
				Sphere* s = (Sphere*)Vector_Get(&spheres,k);

				v00.y -= FAKTOR * Sphere_Gravity(s,Vec3D_New(v00.x,-1.0f,v00.z));
				v10.y -= FAKTOR * Sphere_Gravity(s,Vec3D_New(v10.x,-1.0f,v10.z));
				v01.y -= FAKTOR * Sphere_Gravity(s,Vec3D_New(v01.x,-1.0f,v01.z));
				v11.y -= FAKTOR * Sphere_Gravity(s,Vec3D_New(v11.x,-1.0f,v11.z));
			}

			Tri3D t1 = { .p = { v00,v11,v10 }, .c = WHITE };
			Tri3D_CalcNorm(&t1);
			Tri3D_ShadeNorm(&t1,Vec3D_New(0.5f,0.6f,0.7f));

			Tri3D t2 = { .p = { v00,v01,v11 }, .c = WHITE };
			Tri3D_CalcNorm(&t2);
			Tri3D_ShadeNorm(&t2,Vec3D_New(0.5f,0.6f,0.7f));

			Vector_Push(&world.trisIn,&t1);
			Vector_Push(&world.trisIn,&t2);
		}
	}

	Clear(LIGHT_BLUE);

	World3D_update(&world,cam.p,(Vec2){ GetWidth(),GetHeight() });

	for(int i = 0;i<world.trisOut.size;i++){
		Tri3D* t = (Tri3D*)Vector_Get(&world.trisOut,i);

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