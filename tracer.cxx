/*********************************************************************
 *  CMPSC 457                                                        *
 *  Template code for HW 5                                           *
 *  April 24, 2013                                                   *
 *  Charles Summerscales                                             *
 *  cos5311@psu.edu                                                  *
 *                                                                   *
 *                                                                   *
 *  Description:                                                     *
 *                                                                   *
 *  Basic Ray Tracer program for spheres and triangles.              *
 *********************************************************************/  


#include <cmath>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cfloat>
#include <sstream>


using namespace std;

void SphereBuild(string fileName);
void TriangleBuild(string fileName);
void Tracer();

int num_sph = 0, num_tri = 0; // <-- counter for objects in file

//color structure, easier to keep track of
struct RGB
{
	double r;
	double g;
	double b;
};


struct Vector
{
  double x, y, z;

  Vector(double x=0, double y=0, double z=0)
    : x(x), y(y), z(z) {}

  Vector operator+ (const Vector& v) const
  { return Vector(x+v.x, y+v.y, z+v.z); }

  Vector operator- (const Vector& v) const
  { return Vector(x-v.x, y-v.y, z-v.z); }
  
  Vector operator* (double d) const
  { return Vector(x*d, y*d, z*d); }

  Vector operator* (const Vector& v) const
  { return Vector(x*v.x, y*v.y, z*v.z); }

  // normalize this vector
  Vector& norm() 
  { return *this = *this * (1 / sqrt(x*x + y*y + z*z)); }

  // dot product
  double dot(const Vector& v) const
  { return x*v.x + y*v.y + z*v.z; }

  // cross product
  Vector cross(const Vector& v) const
  { return Vector(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
};


// Not the best way but it works
typedef Vector Point;
typedef Vector Color;


struct Ray
{
  Point o;   // origin of the ray
  Vector d;  // direction of the ray
           
  Ray (Point o, Vector d) : o(o), d(d) {} 
};



struct Sphere
{
  double r;        // radius
  Point p;         // center
  Color c;         // color
  RGB ** tex;	   // texture raster array
  int imgX;			// texture width
  int imgY;			//texture height

  Sphere(double r, Point p, Color c)
    : r(r), p(p), c(c){}

  Sphere (): r(r), p(p), c(c){}
  // compute the intersection of the ray with this sphere
  double intersect(const Ray& ray) const
	{
		// solve (d.d)t^2 + 2d.(o-c)t + (o-c).(o-c) - R^2 = 0 for t
		Vector d = ray.d;
		Vector o = ray.o;
		Vector camSphDist = o - p;		
		double t1, t2;		
		double discrm = (d.dot(camSphDist))*(d.dot(camSphDist)) - (d.dot(d) * (camSphDist.dot(camSphDist) - r*r));
		
		//compute smallest 't' of hit point
		if (discrm >= 0) 
		{			
			t1 = ((-d.dot(camSphDist)) + sqrt(discrm)) / (d.dot(d));
			t2 = ((-d.dot(camSphDist)) - sqrt(discrm)) / (d.dot(d));
			if (t2 < t1) t1 = t2;
			return (t1);
		}
		else return 0;
	}

  // compute the surface unit normal vector at point v on this sphere
  Vector normal(Point& v)
  {
    return Vector(v.x-p.x, v.y-p.y, v.z-p.z).norm();
  }
};


struct Triangle
{
  Point p0, p1, p2;         // vertices
  Vector u0, u1, u2, surfNorm;		//texture coords
  double texU;
  double texV;				// texture mapping coords
  RGB ** tex;	   // texture raster array
  int imgX;			//texture file width
  int imgY;			//texture file height

  Triangle(Point p0, Point p1, Point p2, Vector u0, Vector u1, Vector u2)
    : p0(p0), p1(p1), p2(p2), u0(u0), u1(u1), u2(u2) {}

  Triangle() : p0(p0), p1(p1), p2(p2), u0(u0), u1(u1), u2(u2) {}

  // compute the intersection of the ray with this triangle (needs REVISED)
  double intersect(const Ray& ray)
  {
    // hit routine
	double alpha, beta, gamma, t, a, b, c, d, e, f, g, h, i, j, k, l, m;
	Vector eye = ray.o;
	Vector dist = ray.d;
	
	a = p0.x - p1.x;	d = p0.x - p2.x;	g = dist.x;		j = p0.x - eye.x;
	b = p0.y - p1.y;	e = p0.y - p2.y;	h = dist.y;		k = p0.y - eye.y;
	c = p0.z - p1.z;	f = p0.z - p2.z;	i = dist.z;		l = p0.z - eye.z;
	
	m = a*(e*i - h*f) + b*(g*f - d*i) + c*(d*h - e*g);
	beta = (j*(e*i - h*f) + k*(g*f - d*i) + l*(d*h - e*g))/m;
	gamma = (i*(a*k - j*b) + h*(j*c - a*l) + g*(b*l - k*c))/m;	
	t = (f*(a*k - j*b) + e*(j*c - a*l) + d*(b*l - k*c))/m;
	alpha = 1 - beta - gamma;	
	
	if(beta > 0 && gamma > 0 && beta + gamma < 1) 
	{
		//find texture mapping
		texV = u0.x + beta*(u1.x - u0.x) + gamma*(u2.x - u0.x);
		texU = u0.y + beta*(u1.y - u0.y) + gamma*(u2.y - u0.y);
		return t;
	}
	else return 0;
  }
};
 
//object builders, not efficient but it works
Sphere * spheres = new Sphere[1000];
Triangle * triangles = new Triangle[1000];
string textureSph[1000];
string textureTri[1000];

//build sphere objects
void SphereBuild (string fileName)
{
	ifstream sphereFile(fileName.c_str());
	double cx, cy, cz, radius, r, g, b;
	string line;	
		
	if (sphereFile.is_open())
	{
		while (!sphereFile.eof())
		{		
			//retrieve file input
			getline(sphereFile, line);	
			std::istringstream iss;
			iss.str(line);
			iss >> cx >> cy >> cz >> radius >> r >> g >> b >> textureSph[num_sph];
			spheres[num_sph] = (Sphere(radius, Point(cx, cy, cz), Color(r, g, b)));	
						
			//start of texture block
			if (textureSph[num_sph] != "none")
			{
				ifstream textFile(textureSph[num_sph].c_str());
				string textline;
				if (textFile.is_open())
				{
					getline(textFile, textline);	
					std::istringstream tiss;
					tiss.str(textline);
					getline(textFile, textline);
					getline(textFile, textline);
					tiss.str(textline);
					tiss >> spheres[num_sph].imgY >> spheres[num_sph].imgX;					
					getline(textFile, textline);
					tiss.str(textline);

					//start reading values to raster array
					spheres[num_sph].tex = new RGB*[spheres[num_sph].imgY];
					for (int k = 0; k < spheres[num_sph].imgY; k++) {spheres[num_sph].tex[k]= new RGB[spheres[num_sph].imgX];};						
					for (int i = 0; i < spheres[num_sph].imgX; i++)
					{
						for(int j = 0; j < spheres[num_sph].imgY; j++)
						{
							//store r, g, b values for each pixel
							getline(textFile, textline); 
							spheres[num_sph].tex[j][i].r = std::atoi(textline.c_str());
							getline(textFile, textline);
							spheres[num_sph].tex[j][i].g = std::atoi(textline.c_str());
							getline(textFile, textline);
							spheres[num_sph].tex[j][i].b = std::atoi(textline.c_str());
						}
					}						
					textFile.close();
				}				
			}
			//end of texture block			
			num_sph++;
		}
	}
	sphereFile.close();	
}


//build triangle objects
void TriangleBuild (string fileName)
{
	ifstream triangleFile(fileName.c_str());
	double x0, y0, z0, x1, y1, z1, x2, y2, z2, u0, v0, u1, v1, u2, v2;
	string line;
	
	if (triangleFile.is_open())
	{
		while (!triangleFile.eof())
		{
			//retrieve file input
			getline(triangleFile, line);
			std::istringstream iss;
			iss.str(line);
			iss >> x0 >> y0 >> z0 >> u0 >> v0 >> x1 >> y1 >> z1 >> u1 >> v1 >> x2 >> y2 >> z2 >> u2 >> v2 >> textureTri[num_tri];
			triangles[num_tri] = (Triangle(Point(x0, y0, z0), Point(x1, y1, z1), Point(x2, y2, z2), Vector(u0, v0, 0.0), Vector(u1, v1, 0.0), Vector(u2, v2, 0.0)));
			triangles[num_tri].surfNorm = (triangles[num_tri].p1 - triangles[num_tri].p0).cross(triangles[num_tri].p2 - triangles[num_tri].p0);	
			
			//start of texture block
			if (textureTri[num_tri] != "none")
			{
				ifstream textFile(textureTri[num_tri].c_str());
				string textline;
				if (textFile.is_open())
				{
					getline(textFile, textline);	
					std::istringstream tiss;
					tiss.str(textline);
					getline(textFile, textline);
					getline(textFile, textline);
					tiss.str(textline);
					tiss >> triangles[num_tri].imgY >> triangles[num_tri].imgX;
					getline(textFile, textline);
					tiss.str(textline);

					//start reading values to raster array
					triangles[num_tri].tex = new RGB*[triangles[num_tri].imgY];
					for (int k = 0; k < triangles[num_tri].imgY; k++) {triangles[num_tri].tex[k]= new RGB[triangles[num_tri].imgX];};						
					for (int i = 0; i < triangles[num_tri].imgX; i++)
					{
						for(int j = 0; j < triangles[num_tri].imgY; j++)
						{
							//store r, g, b values for each pixel
							getline(textFile, textline); 
							triangles[num_tri].tex[j][i].r = std::atoi(textline.c_str());
							getline(textFile, textline);
							triangles[num_tri].tex[j][i].g = std::atoi(textline.c_str());
							getline(textFile, textline);
							triangles[num_tri].tex[j][i].b = std::atoi(textline.c_str());
						}
					}						
					textFile.close();
				}				
			}
			//end of texture block			
			num_tri++;
		}
	}
	triangleFile.close();
}

//ray tracing implementation
void Tracer(string fileName, Vector eye, Vector direction, Vector light, Vector background, double fov, int nx, int ny, bool tri, bool sph)
{	
	ofstream outFile (fileName.c_str());
	RGB **newColor = new RGB*[ny];
	for (int k = 0; k < ny; k++) {newColor[k]= new RGB[nx];};
	
	RGB originalColor;
	RGB bgColor; bgColor.r = background.x; bgColor.g = background.y; bgColor.b = background.z;
	
	//build ppm file
	outFile << "P3\n" << nx << " " << ny << "\n" << "255\n";
	
	//calculate image to screen coordinates
	double dd = 1;
	double h = 2.0 * dd * tan((double(fov)*M_PI/180.0)/2.0);
	double w = (double(nx)/double(ny)) * h;
	double t = 0, minT = 0, u = 0, v = 0, theta, phi;
	int objHit = 0;
	Point p; 
	Vector d;
	
	//arbitrary view configuration (not completed!)
	//Vector wHat = direction.norm();
	//Vector uHat = (up.cross(wHat)).norm();
	//Vector vHat = wHat.cross(uHat);
	
	//run through translation, hit routine, and shading
	//loop for x
	for (int i = 0; i < nx; i++)
	{
		//loop for y
		for (int j = 0; j < ny; j++)
		{
		//transform p(pixel) to p'(on film)		
		p.x = (w/double(nx)) * (j - (double(nx)/2.0)); p.y = (w/double(nx)) * (i - (double(ny)/2.0)); p.z = eye.z + (-dd);
		
		//arbitrary view transform (not working right)
		//p.x = uHat.x*p.x + vHat.x*p.y + wHat.x*0 + eye.x; 
		//p.y = uHat.y*p.x + vHat.y*p.y + wHat.y*0 + eye.y; 
		//p.z = uHat.z*p.x + vHat.z*p.y + wHat.z*0 + eye.z;
		
		d = p - eye;
		d.norm();
		Ray r(eye, d);
				
		//determine hit point and find ray color
		if (sph) 
			{
				//cycle through ALL sphere objects and look for hit point
				for (int m = 0; m < num_sph; m++)
				{
					t = spheres[m].intersect(r);
					if (minT == 0) 
					{
						minT = t;
						objHit = m;
					}
					else if ((t > 0) &&(t < minT)) 
					{
						minT = t;
						objHit = m;
					}
				}
				
				if (minT > 0) //did ray hit object?
				{
					Point phit = eye + (d*minT); // <-- hit point
					
					//texture mapping
					if (textureSph[objHit] != "none") 
					{
						//calculate theta and phi
						theta = acos((phit.z - spheres[objHit].p.z)/spheres[objHit].r);;
						phi = atan((phit.y - spheres[objHit].p.y)/(phit.x - spheres[objHit].p.x));
							if (phi < 0.0) phi = phi + 2.0*M_PI;
						//calculate texture mapping coordinates 
						u = (1.0/(M_PI))* theta;
						v = ((1.0)/(2.0 * M_PI)) * phi; 
						u = floor(u*spheres[objHit].imgX);
						v = floor(v*spheres[objHit].imgY);
						//set color to texture color
						originalColor.r = spheres[objHit].tex[int(v)][int(u)].r/ 255.0;
						originalColor.g = spheres[objHit].tex[int(v)][int(u)].g/ 255.0;
						originalColor.b = spheres[objHit].tex[int(v)][int(u)].b/ 255.0;
					}
					//solid color
					else {originalColor.r = spheres[objHit].c.x; originalColor.g = spheres[objHit].c.y; originalColor.b = spheres[objHit].c.z;}
					
					//lambertian shading
					Vector nhat = phit - spheres[objHit].p;
					nhat.norm();
					Vector lhat = light - phit;
					lhat.norm();
					newColor[j][i].r = originalColor.r * max(0.0, nhat.dot(lhat));
					newColor[j][i].g = originalColor.g * max(0.0, nhat.dot(lhat));
					newColor[j][i].b = originalColor.b * max(0.0, nhat.dot(lhat));				
				}
			}
		if (tri)
			{
				//cycle through ALL triangle objects and look for hit point
				for (int m = 0; m < num_tri; m++)
				{
					t = triangles[m].intersect(r);
					if (minT == 0) 
					{
						minT = t;
						objHit = m;
					}
					else if ((t < minT) && (t != 0))
					{
						minT = t;
						objHit = m;
					}
				}			
				
				if (minT < 0) //did ray hit object?
				{
					Point phit = eye + (d*minT); //<-- determine hit point
					
					//texture mapping
					if (textureTri[objHit] != "none") 
					{	
						originalColor.r = triangles[objHit].tex[int(triangles[objHit].texV)][int(triangles[objHit].texU)].r/ 255.0;
						originalColor.g = triangles[objHit].tex[int(triangles[objHit].texV)][int(triangles[objHit].texU)].g/ 255.0;
						originalColor.b = triangles[objHit].tex[int(triangles[objHit].texV)][int(triangles[objHit].texU)].b/ 255.0;
					}					
					//solid color for testing
					else {originalColor.r = 0.4; originalColor.g = 0.6; originalColor.b = 0.1;}
					
					//lambertian shading
					Vector nhat = triangles[objHit].surfNorm;
					nhat.norm();
					Vector lhat = light - phit;
					lhat.norm();
					newColor[j][i].r = originalColor.r * max(0.0, abs(nhat.dot(lhat)));
					newColor[j][i].g = originalColor.g * max(0.0, abs(nhat.dot(lhat)));
					newColor[j][i].b = originalColor.b * max(0.0, abs(nhat.dot(lhat)));
				}
			}
			
		//background color for no hit point
		if (minT == 0) outFile << (bgColor.r * 255.0) << " " << (bgColor.g * 255.0) << " " << (bgColor.b * 255.0) << "  "; 
		//hit point color otherwise
		else outFile << int(newColor[j][i].r * 255.0) << " " << int(newColor[j][i].g * 255.0) << " " << int(newColor[j][i].b * 255.0) << "  ";	
		
		//reset t back to 0				
		minT = 0;
		}
	}
	outFile.close();
}


int main(int argc, char* argv[])
{  
  //i/o filenames
  string inFileName = argv[3];
  string outFileName = argv[4];  
  string line, rayParams[4], sphFileName, triFileName;  
    
  //converts user entered dimensions to integers
  int nx = std::atoi(argv[1]), ny = std::atoi(argv[2]), hitObj = 0;
  bool sph = false, tri = false;
  double angle;
  Vector lfrom;
  Vector lat;
  Vector vup;
  Vector light;
  Vector bground;
  
  ifstream inFile(inFileName.c_str()); 
  
  if (inFile.is_open())
  {
	while (!inFile.eof())
	{
		//retrieve file input
		getline(inFile, line);		
		std::istringstream iss;
		iss.str(line);
		iss >> rayParams[0] >> rayParams[1] >> rayParams[2] >> rayParams[3];
				
		//read inputfile into parameters
		if (rayParams[0] == "lookfrom")
		{
			//assemble 'e' vector			
			lfrom.x = std::atoi(rayParams[1].c_str());
			lfrom.y = std::atoi(rayParams[2].c_str());
			lfrom.z = std::atoi(rayParams[3].c_str());
		}
		else if (rayParams[0] == "lookat")
		{
			//store rest in vector			
			lat.x = std::atoi(rayParams[1].c_str());
			lat.y = std::atoi(rayParams[2].c_str());
			lat.z = std::atoi(rayParams[3].c_str());
		}
		else if (rayParams[0] == "vfov")
		{
			//vertical field of view
			angle = std::atoi(rayParams[1].c_str());
		}
		else if (rayParams[0].c_str() == "vup")
		{
			//view up for arbitrary view points			
			vup.x = std::atoi(rayParams[1].c_str());
			vup.y = std::atoi(rayParams[2].c_str());
			vup.z = std::atoi(rayParams[3].c_str());
		}
		else if (rayParams[0] == "light")
		{
			//store rest in vector
			light.x = std::atoi(rayParams[1].c_str());
			light.y = std::atoi(rayParams[2].c_str());
			light.z = std::atoi(rayParams[3].c_str());
		}
		else if (rayParams[0] == "background")
		{
			//store rest in vector
			bground.x = std::atoi(rayParams[1].c_str());
			bground.y = std::atoi(rayParams[2].c_str());
			bground.z = std::atoi(rayParams[3].c_str());
		}
		else if (rayParams[0] == "spheres")
		{
			//build spheres with filename
			sphFileName = rayParams[1].c_str();
			SphereBuild(sphFileName);
			sph = true;
		}
		else if (rayParams[0] == "triangles")
		{
			//build triangle with filename
			triFileName = rayParams[1].c_str();
			TriangleBuild(triFileName);
			tri = true;
		}		
	}
  }
  inFile.close();  

  //call tracer method with assembled file parameters
  Tracer(outFileName, lfrom, lat, light, bground, angle, nx, ny, tri, sph);
  
  return 0;
};