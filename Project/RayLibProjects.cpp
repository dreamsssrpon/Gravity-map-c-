#include "RayLibProjects.h"
#include <raylib.h>
#include "external_data/glm/glm/glm.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

using namespace std;
using namespace glm;

int quad_dist = false;

class GravityBody {
public:
	float gravity;
	vec2 pos;

	float r;

	GravityBody(){}
	GravityBody(float g,vec2 p){
		pos = p;
		gravity = g;
	}

	void show() {
		//DrawCircle(pos.x, pos.y,r,RED);
	}
};

vector<GravityBody> bodys;

class TrailBody {
public:
	float mass;
	vec2 speed,pos;

	vector<vec2> trail;

	TrailBody() {}
	TrailBody(vec2 p,float m) {
		pos = p;
		mass = m;
		speed = vec2(0, 0);
	}	
	void computing() {
		vec2 force(0,0);

		for (int i = 0;i < bodys.size();++i) {
			float d = distance(pos, bodys[i].pos);
			if (d <= 40) {
				speed = vec2(0);
			}
			if (quad_dist == 0) {
				force += normalize(bodys[i].pos - pos) * vec2(bodys[i].gravity / (d * d));
			}
			else {
				force += normalize(bodys[i].pos - pos) * vec2(bodys[i].gravity / (d*100));
			}
		}

		speed += force/vec2(mass);
	}
	void comp() {
		for (int i = 0; i < 1800;++i) {
			computing();
			pos += speed * vec2(0.1f);

			trail.push_back(pos);
		}
	}
	void update() {
		pos += speed * vec2(0.1f);

		trail.push_back(pos);

		if (trail.size() >= 255) {
			trail.erase(trail.begin());
		}
	}
	void draw() {
		DrawCircle(pos.x, pos.y, 10, BLUE);

		for (int i = 0;i + 1 < trail.size();++i) {
			Color c = WHITE;
			c.a -= (trail.size() - i);
			DrawLineEx(Vector2(trail[i].x, trail[i].y), Vector2(trail[i + 1].x, trail[i + 1].y), 4,c);
		}

		float l = length(speed);
		vec2 norm = normalize(speed)*l;
			
		DrawLineEx(Vector2(pos.x+0.1, pos.y+0.1), Vector2(pos.x + (norm.x)+0.1, pos.y + (norm.y)+0.1),5,GREEN);
	}
};
class timer1 {
public:
	double st, interval;
	timer1() {}
	timer1(double i){
		interval = i;
		st = 0;
	}
	void start() {
		st = GetTime();
	}
	bool check() {
		double curr = GetTime();
		if (curr-st >= interval) {
			st = curr;
			return true;
		}return false;
	}
};

Shader sh;
vector<vec2> poss(bodys.size(), vec2(0, 0));
vector<float> gs(bodys.size(), 0);
RenderTexture2D txt;

void TakeData();

void init_shader_data() {

	txt = LoadRenderTexture(800, 800);

	string shader = R"( 
	#version 330 core

	in vec2 fragTexCoord;
	in vec4 fragColor;	

	const int size = 32;

	uniform int boyd_count;
	uniform vec2[size] bds;
	uniform float[size] gravity;

	uniform int circls;
	uniform int is_quad;

	out vec4 Vco;

	float lenght(vec2 v){
		return (v.x*v.x)+(v.y*v.y);
	}

	vec3 force_to_gradient(float force2,int count){
		float step = (1.0/float(count));
	
		int step_count = int(floor(force2/step));

		force2 = step*step_count;

		return vec3(force2,0,1-force2);
	}		
	vec3 force_to_gradient(float force2,float step,int st){		
		force2 = float((force2/step))*step;

		return vec3(force2,0,1-force2);
	}	
	
	void main() {
		vec2 force = {0,0};
		vec2 pos =  gl_FragCoord.xy;

		for(int i = 0; i < boyd_count;++i){
			float dist = distance(bds[i],pos);

			if(is_quad==0){
				force += (normalize(bds[i]-pos)*(gravity[i]/((dist*dist)+1)));	
			}
			else{
				force += (normalize(bds[i]-pos)*(gravity[i]/((dist*100)+1)));	
			}	
		}

		vec3 rgb;
		float lnght = clamp(lenght(force)*100,0.0,1.0);

		rgb = force_to_gradient(lnght,circls);

		Vco = vec4(rgb,1.0);
	}

	)";
    sh = LoadShaderFromMemory(NULL, shader.data());
	TakeData();
};

GravityBody template_body;

int circls;

void loadGameSettings(string fil) {
	std::ifstream file(fil.data());

	quad_dist = 1;
	circls = 17;
	float temp_grav = 900;

	if (!file.is_open()) {
		template_body = GravityBody(temp_grav, vec2(0, 0));
		cout << "file not open\n";
		return;
	}

	file >> temp_grav >> quad_dist >> circls;

	template_body = GravityBody(temp_grav, vec2(0, 0));

	file.close();
}

void TakeData() {
	poss = vector<vec2>(bodys.size(), vec2(0, 0));
	gs = vector<float>(bodys.size(), 0);

	for (int i = 0; i < bodys.size();++i) {
		gs[i] = bodys[i].gravity;
		poss[i] = bodys[i].pos;
	}

	int loc = GetShaderLocation(sh, "boyd_count");
	int size = bodys.size();
	SetShaderValue(sh, loc, &size, SHADER_UNIFORM_INT);

	loc = GetShaderLocation(sh, "bds");
	SetShaderValueV(sh, loc, poss.data(), SHADER_UNIFORM_VEC2,size);

	loc = GetShaderLocation(sh, "gravity");
	SetShaderValueV(sh, loc, gs.data(), SHADER_UNIFORM_FLOAT, size);

	loc = GetShaderLocation(sh, "circls");
	SetShaderValue(sh, loc, &circls, SHADER_UNIFORM_INT);

	loc = GetShaderLocation(sh, "is_quad");
	SetShaderValue(sh, loc, &quad_dist, SHADER_UNIFORM_INT);
}

void GravityMap() {		
	TakeData();

	BeginTextureMode(txt);
	ClearBackground(BLACK);

	BeginShaderMode(sh);

	DrawRectangle(0, 0, 800, 800, WHITE);

	EndShaderMode();
	EndTextureMode();
}

int main()
{
	InitWindow(800, 800, "Truster simulation");
	SetTargetFPS(120);

	bool is_pause = false;

	//loadGameSettings("C:\\Users\\maxim\\source\\repos\\RayLibProjects\\RayLibProjects\\Game_settings.txt");
	loadGameSettings("Game_settings.txt");

	vec2 pos(400, 300);
	vec2 dir(0, 3);
	string label;

	GravityBody bod(template_body);
	bod.r = 30;
	bodys.push_back(bod);

	TrailBody bd(vec2(400,300), 1);
	bd.speed.x += 4.5;
	//bd.comp();

	init_shader_data();

	while (!WindowShouldClose()) {

		if (!is_pause) {
			bodys.back().pos = (vec2(GetMousePosition().x, GetMousePosition().y));
		}

		GravityMap();

		BeginDrawing();		
		ClearBackground(BLACK);

		DrawTextureRec(txt.texture,{0,0,800,800}, Vector2(0, 0), WHITE);

		for (int i = 0; i < bodys.size();++i) {
			bodys[i].show();
		}

		if (!is_pause) {
			bd.computing();
			bd.update();
		}
		bd.draw();

		if (IsKeyPressed(KEY_W)) {
			pos.y += 7;
		}
		if (IsKeyPressed(KEY_A)) {
			pos.x -= 7;
		}
		if (IsKeyPressed(KEY_D)) {
			pos.x += 7;
		}
		if (IsKeyPressed(KEY_S)) {
			pos.y -= 7;
		}

		if (IsKeyPressed(KEY_UP)) {
			dir.y += 7;
		}
		if (IsKeyPressed(KEY_LEFT)) {
			dir.x -= 7;
		}
		if (IsKeyPressed(KEY_RIGHT)) {
			dir.x += 7;
		}
		if (IsKeyPressed(KEY_DOWN)) {
			dir.y -= 7;
		}

		if (IsKeyPressed(KEY_E)) {
			is_pause = !is_pause;
		}

		if (IsMouseButtonPressed(1)||is_pause) {
			bd = TrailBody(pos,2);
			bd.speed = dir;
		}if (IsMouseButtonPressed(2)) {
			bod = GravityBody(400.f, vec2(0, 0));
			bod.r = 20;
			bodys.push_back(bod);
		}
		if (IsMouseButtonPressed(0)) {
			bodys.pop_back();
		}

		label = "";
		label += to_string(dir.x);
		label += " ";
		label += to_string(dir.y);
		DrawText(label.data(), 0, 0,10,WHITE);
		label = "";
		label += to_string(pos.x);
		label += " ";
		label += to_string(pos.y);
		DrawText(label.data(), 0, 20, 10, WHITE);

		EndDrawing();
	}

	return 0;
}
