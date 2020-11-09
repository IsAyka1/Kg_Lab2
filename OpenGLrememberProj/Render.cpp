#include "Render.h"

#include <sstream>
#include <iostream>
#include <Array>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;
bool alphaMode = false;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'A')
	{
		alphaMode = !alphaMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	delete[] texarray;

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}
std::array<double, 2> Normaliz(double x, double y) {
	return { (x - 7) / 14, (y + 2) / 18 };
}

double* Normal(const double(&A)[3], const double(&B)[3], const double(&C)[3], double* N) {
	double A1[3], C1[3];
	for (int i = 0; i < 3; i++) {
		A1[i] = B[i] - A[i];
		C1[i] = B[i] - C[i];
	}
	N[0] = (A1[1] * C1[2]) - (C1[1] * A1[2]);
	N[1] = (C1[0] * A1[2]) - (A1[0] * C1[2]);
	N[2] = (A1[0] * C1[1]) - (C1[0] * A1[1]);
	double len = sqrt(N[0] * N[0] + N[1] * N[1] + N[2] * N[2]);
	for (int i = 0; i < 3; i++) {
		N[i] /= len;
	}
	return N;
}

#pragma region Figure
void Curcle(int z = 0) {
	double O[] = { 1, 9.5, z };
	double D[] = { 6, 10, z };
	//double E[] = { -4, 9, z };

	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0.75, 0.0, 0.25);
	glVertex3dv(O);
	glVertex3dv(D);
	for (double i = 6, r = 5.025; i <= 186; i++) { // atan(0.1); 180 + atan(0.1)
		double x = r * cos(i * M_PI / 180) + O[0];
		double y = r * sin(i * M_PI / 180) + O[1];
		double New[] = { x, y, z };
		glVertex3dv(New);
		//glVertex3dv(E);
	}

	glEnd();
}

void CurcleRotate(int z, int f, double alpha = 1) {
	glBegin(GL_TRIANGLE_FAN);
	glColor4d(0.75, 0.0, 0.25, alpha);
	double O[] = { 1, 9.5, z };
	auto N = Normaliz(O[0], O[1]).data();
	glTexCoord3d(N[0], N[1], 3);
	double x = O[0] * cos(f * M_PI / 180) - O[1] * sin(f * M_PI / 180);
	double y = O[0] * sin(f * M_PI / 180) + O[1] * cos(f * M_PI / 180);
	O[0] = x; O[1] = y;
	glVertex3dv(O);
	double D[] = { 6, 10, z };
	N = Normaliz(D[0], D[1]).data();
	glTexCoord3d(N[0], N[1], 3);
	x = D[0] * cos(f * M_PI / 180) - D[1] * sin(f * M_PI / 180);
	y = D[0] * sin(f * M_PI / 180) + D[1] * cos(f * M_PI / 180);
	D[0] = x; D[1] = y;
	//double E[] = { -4, 9, z };
	glVertex3dv(D);

	for (double i = 6, r = 5.025; i <= 186; i++) { // atan(0.1); 180 + atan(0.1)
		double x = r * cos(i * M_PI / 180) + 1;
		double y = r * sin(i * M_PI / 180) + 9.5;
		double newX = x * cos(f * M_PI / 180) - y * sin(f * M_PI / 180);
		double newY = x * sin(f * M_PI / 180) + y * cos(f * M_PI / 180);
		N = Normaliz(x, y).data();
		glTexCoord3d(N[0], N[1], 3);
		glVertex3d(newX, newY, z);
		//glVertex3d(x, y, z);
		//glVertex3dv(E);
	}
	glEnd();
}

void CurcleBackRotate(int f) {
	double D[] = { 6, 10, 0 };
	double D1[] = { 6, 10, 3 };
	glBegin(GL_QUAD_STRIP);
	glColor3d(0.5, 0.0, 0.5);
	bool fl = true;
	double r = 5.025;
	D1[0] = D[0] * cos(f * M_PI / 180) - D[1] * sin(f * M_PI / 180);
	D1[1] = D[0] * sin(f * M_PI / 180) + D[1] * cos(f * M_PI / 180);
	double x = r * cos(6 * M_PI / 180) + 1;
	double y = r * sin(6 * M_PI / 180) + 9.5;
	double newX = x * cos(f * M_PI / 180) - y * sin(f * M_PI / 180);
	double newY = x * sin(f * M_PI / 180) + y * cos(f * M_PI / 180);
	double N[] = { 0,0,0 };
	Normal(D, D1, { newX, newY, 3 }, N);
	glNormal3dv(N);
	glVertex3dv(D);
	glVertex3dv(D1);
	glVertex3d(x, y, 0);
	glVertex3d(newX, newY, 3);
	for (double i = 6; i < 187; i++) { // atan(0.1); 180 + atan(0.1)
		if (fl) { 
			glColor3d(0.5, 0, 0.5); fl = false; 
		}
		else {
			glColor3d(0.4, 0, 0.6); fl = true;
		}
		x = r * cos(i * M_PI / 180) + 1;
		y = r * sin(i * M_PI / 180) + 9.5;
		newX = x * cos(f * M_PI / 180) - y * sin(f * M_PI / 180);
		newY = x * sin(f * M_PI / 180) + y * cos(f * M_PI / 180);
		if (i + 1 != 187)
		{
			double secX = r * cos((i + 1) * M_PI / 180) + 1;
			double secY = r * sin((i + 1) * M_PI / 180) + 9.5;
			double secNewX = secX * cos(f * M_PI / 180) - secY * sin(f * M_PI / 180);
			double secNewY = secX * sin(f * M_PI / 180) + secY * cos(f * M_PI / 180);
			double N[] = { 0,0,0 };
			Normal({ x, y, 0 }, { newX, newY, 3 }, {secNewX, secNewY, 3}, N);
			glNormal3dv(N);
			glVertex3d(x, y, 0);
			glVertex3d(newX, newY, 3);
			glVertex3d(secX, secY, 0);
			glVertex3d(secNewX, secNewY, 3);
		}
	}
	glEnd();
}

void HoleBackRotate(int f) {
	double O[] = { -7.8, -8.5 };
	double A[] = { 0, 0, 0 };
	double A1[] = { 0, 0, 3 };
	double r = 11.55;
	A1[0] = A[0] * cos(f * M_PI / 180) - A[1] * sin(f * M_PI / 180);
	A1[1] = A[0] * sin(f * M_PI / 180) + A[1] * cos(f * M_PI / 180);
	double x = r * cos(48 * M_PI / 180) + O[0];
	double y = r * sin(48 * M_PI / 180) + O[1];
	double newX = x * cos(f * M_PI / 180) - y * sin(f * M_PI / 180);
	double newY = x * sin(f * M_PI / 180) + y * cos(f * M_PI / 180);
	double N[] = { 0,0,0 };
	Normal({ x, y, 0 }, A, A1, N);
	glNormal3dv(N);
	glBegin(GL_QUAD_STRIP);
	glColor3d(0.5, 0.5, 0);
	glVertex3dv(A);
	glVertex3dv(A1);
	glVertex3d(x, y, 0);
	glVertex3d(newX, newY, 3);
	for (double i = 48; i < 97; i++) {
		double x = r * cos(i * M_PI / 180) + O[0];
		double y = r * sin(i * M_PI / 180) + O[1];
		double newX = x * cos(f * M_PI / 180) - y * sin(f * M_PI / 180);
		double newY = x * sin(f * M_PI / 180) + y * cos(f * M_PI / 180);
		if (i + 1 != 97)
		{
			double secX = r * cos((i + 1) * M_PI / 180) + O[0];
			double secY = r * sin((i + 1) * M_PI / 180) + O[1];
			double secNewX = secX * cos(f * M_PI / 180) - secY * sin(f * M_PI / 180);
			double secNewY = secX * sin(f * M_PI / 180) + secY * cos(f * M_PI / 180);
			double N[] = { 0,0,0 };
			Normal( { newX, newY, 3 }, { x, y, 0 }, { secX, secY, 0 }, N);
			glNormal3dv(N);
			glVertex3d(x, y, 0);
			glVertex3d(newX, newY, 3);
			glVertex3d(secX, secY, 0);
			glVertex3d(secNewX, secNewY, 3);
		}
	}
	glEnd();
}

void Hole(int z) {
	double F[] = { -3, 5, z };
	double A[] = { 0, 0, z };
	double G[] = { -9, 3, z };
	double M[] = { -3, 2, z };
	double O[] = { -7.8, -8.5, z };

	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0.75, 0.25, 0);
	glVertex3dv(F);
	glVertex3dv(A);
	for (double i = 48, r = 11.55; i <= 96; i++) {
		double x = r * cos(i * M_PI / 180) + O[0];
		double y = r * sin(i * M_PI / 180) + O[1];
		double New[] = { x, y, z };
		glVertex3dv(New);
	}
	glVertex3dv(G);
	glEnd();
}

void HoleRotate(int z, int f, double alpha = 1) {
	glBegin(GL_TRIANGLE_FAN);
	glColor4d(0.75, 0.25, 0, alpha);
	double F[] = { -3, 5, z };
	auto N = Normaliz(F[0], F[1]).data();
	glTexCoord3d(N[0], N[1], 3);
	double x = F[0] * cos(f * M_PI / 180) - F[1] * sin(f * M_PI / 180);
	double y = F[0] * sin(f * M_PI / 180) + F[1] * cos(f * M_PI / 180);
	F[0] = x; F[1] = y;
	glVertex3dv(F);
	double A[] = { 0, 0, z };
	N = Normaliz(A[0], A[1]).data();
	glTexCoord3d(N[0], N[1], 3);
	x = A[0] * cos(f * M_PI / 180) - A[1] * sin(f * M_PI / 180);
	y = A[0] * sin(f * M_PI / 180) + A[1] * cos(f * M_PI / 180);
	A[0] = x; A[1] = y;
	glVertex3dv(A);
	double G[] = { -9, 3, z };
	
	double M[] = { -3, 2, z };
	double O[] = { -7.8, -8.5, z };
	//x = O[0] * cos(f * M_PI / 180) - O[1] * sin(f * M_PI / 180);
	//y = O[0] * sin(f * M_PI / 180) + O[1] * cos(f * M_PI / 180);
	//O[0] = x; O[1] = y;
	
	for (double i = 48, r = 11.55; i <= 96; i++) {
		double x = r * cos(i * M_PI / 180) + O[0];
		double y = r * sin(i * M_PI / 180) + O[1];
		double newX = x * cos(f * M_PI / 180) - y * sin(f * M_PI / 180);
		double newY = x * sin(f * M_PI / 180) + y * cos(f * M_PI / 180);
		N = Normaliz(x, y).data();
		glTexCoord3d(N[0], N[1], 3);
		glVertex3d(newX, newY, z);
	}
	N = Normaliz(G[0], G[1]).data();
	glTexCoord3d(N[0], N[1], 3);
	x = G[0] * cos(f * M_PI / 180) - G[1] * sin(f * M_PI / 180);
	y = G[0] * sin(f * M_PI / 180) + G[1] * cos(f * M_PI / 180);
	G[0] = x; G[1] = y;
	glVertex3dv(G);
	glEnd();
}

void Figure60(int f) {
	const int SIZE = 8;
	double Bot[][3] = {
		{ 0, 0, 0 }, //A
		{ 9, 1, 0 }, //B
		{ 3, 5, 0 }, //C
		{ 6, 10, 0 }, //D
		{0, 9.5, 0}, //Help
		{ -4, 9, 0 }, //E
		{ -3, 5, 0 }, //F
		{ -9, 3, 0 }  //G
	};
	double Top[][3] = {
		{ 0, 0, 3 }, //A1
		{ 9, 1, 3 }, //B1
		{ 3, 5, 3 }, //C1
		{ 6, 10, 3 }, //D1
		{0, 9.5, 3}, //Help
		{ -4, 9, 3 }, //E1
		{ -3, 5, 3 }, //F1
		{ -9, 3, 3 }  //G1
	};

	glBegin(GL_QUAD_STRIP);
	bool fl = true;
	for (int i = 0; i < SIZE - 1; i++) {
		double x = Top[i][0] * cos(f * M_PI / 180) - Top[i][1] * sin(f * M_PI / 180);
		double y = Top[i][0] * sin(f * M_PI / 180) + Top[i][1] * cos(f * M_PI / 180);
		if (fl) { glColor3d(0, 0, 1); fl = false; }
		else { glColor3d(0.4, 0, 0.6); fl = true; }
		double N[] = { 0,0,0 };
		Normal(Bot[i], Top[i], Top[i+1], N);
		glNormal3dv(N);
		glVertex3d(x, y, Top[i][2]);
		glVertex3dv(Bot[i]);
		i++;
		x = Top[i][0] * cos(f * M_PI / 180) - Top[i][1] * sin(f * M_PI / 180);
		y = Top[i][0] * sin(f * M_PI / 180) + Top[i][1] * cos(f * M_PI / 180);
		if (fl) { glColor3d(0, 0, 1); fl = false; }
		else { glColor3d(0.4, 0, 0.6); fl = true; }
		glVertex3d(x, y, Top[i][2]);
		glVertex3dv(Bot[i]);
		i--;
	}
	glEnd();
	HoleBackRotate(f);
	
	glBegin(GL_POLYGON);
	glColor3d(0, 1, 0);
	glNormal3d(0, 0, -1);
	for (int i = 0; i < SIZE - 1; i++) {
		glVertex3dv(Bot[i]);
	}
	glEnd();
	Hole(0);
	Curcle(0);
	CurcleBackRotate(f);
	if (alphaMode) {
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	double alpha = 0.5;
	glBegin(GL_POLYGON);
	glColor4d(1, 0, 0, alpha);
	glNormal3d(0, 0, 1);
	for (int i = 0; i < SIZE - 1; i++) {
		double x = Top[i][0] * cos(f * M_PI / 180) - Top[i][1] * sin(f * M_PI / 180);
		double y = Top[i][0] * sin(f * M_PI / 180) + Top[i][1] * cos(f * M_PI / 180);
		auto N  = Normaliz(Top[i][0], Top[i][1]).data();
		glTexCoord3d(N[0], N[1], 3);
		glVertex3d(x, y, Top[i][2]);
	}
	glEnd();
	CurcleRotate(3, f, alpha);
	HoleRotate(3, f, alpha);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
}

#pragma endregion 

void Render(OpenGL* ogl)
{

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут 
	glBindTexture(GL_TEXTURE_2D, texId);
	Figure60(30);
	//===================================
	

	//glColor3d(0.6, 0.6, 0.6);
	//glBegin(GL_TRIANGLE_FAN);

	//double O[] = { -15, 7, 0 };
	//bool f = false;
	//glTexCoord2d(0.5, 0.5);
	//glVertex3dv(O);
	//for (double i = 0, r = 3; i <= 360; i++) {
	//	double x = r * cos(i * M_PI / 180) + O[0];
	//	double y = r * sin(i * M_PI / 180) + O[1];
	//	glTexCoord2d(0.5*cos(i * M_PI / 180)+0.5, 0.5*sin(i * M_PI / 180) + 0.5);
	//	glVertex3d(x, y, O[2]);
	//}
		//glNormal3d(0, 0, 1);
		//glTexCoord2d(-12, 4);
		//glVertex2dv(A);
		//glTexCoord2d(-12, 10);
		//glVertex2dv(B);
		//glTexCoord2d(-18, 10);
		//glVertex2dv(C);
		//glTexCoord2d(-18, 4);
		//glVertex2dv(D);
	
		//glEnd();
		//конец рисования квадратика станкина


	   //Сообщение вверху экрана


		glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
										//(всек матричные операции, будут ее видоизменять.)
		glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
		glLoadIdentity();	  //Загружаем единичную матрицу
		glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

		glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
		glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
		glLoadIdentity();		  //сбрасываем ее в дефолт

		glDisable(GL_LIGHTING);



		GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
		rec.setSize(300, 150);
		rec.setPosition(10, ogl->getHeight() - 150 - 10);


		std::stringstream ss;
		ss << "T - вкл/выкл текстур" << std::endl;
		ss << "L - вкл/выкл освещение" << std::endl;
		ss << "A - вкл/выкл альфаналожение" << std::endl;
		ss << "F - Свет из камеры" << std::endl;
		ss << "G - двигать свет по горизонтали" << std::endl;
		ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
		ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
		ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
		ss << "Параметры камеры: R=" << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;

		rec.setText(ss.str().c_str());
		rec.Draw();

		glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
		glPopMatrix();


		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
}