#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <memory>
#include <limits>

using namespace std;

const double inf = numeric_limits<double>::infinity();

struct color {
  double r, g, b;
  color(double r, double g, double b) : r(r), g(g), b(b) {}
  color() : r(0), g(0), b(0) {}
  color operator* (const double &x) const {
    return color{r*x, g*x, b*x};
  }
};

class Image {
private:
  int r, c;
  vector<vector<color> > im;
public:
  Image(int _r, int _c) {
    r = _r; c = _c;
    im.resize(r);
    for ( int i = 0 ; i < r ; i++ ) {
      im[i].resize(c);
    }
  }

  Image* putpixel(int _r, int _c, color l) {
    im[_r][_c] = l;
    return this;
  }

  color getpixel(int _r, int _c) const {
    return im[_r][_c];
  }

  void normalize() {
    color lowest{inf,inf,inf}, highest{-inf,-inf,-inf};
    for ( int _r = 0 ; _r < r ; _r++ ) {
      for ( int _c = 0 ; _c < c ; _c++ ) {
	color l = getpixel(_r, _c);
	if ( l.r < lowest.r ) lowest.r = l.r;
	if ( l.g < lowest.g ) lowest.g = l.g;
	if ( l.b < lowest.b ) lowest.b = l.b;
	if ( l.r > highest.r ) highest.r = l.r;
	if ( l.g > highest.g ) highest.g = l.g;
	if ( l.b > highest.b ) highest.b = l.b;
      }
    }
    color mult{highest.r - lowest.r,
               highest.g - lowest.g,
               highest.b - lowest.b};
    if (mult.r <= 0) mult.r = 1; // any strictly positive value suffices
    if (mult.g <= 0) mult.g = 1; // since this is only used to prevent a
    if (mult.b <= 0) mult.b = 1; // zero divide when all values are same
    for ( int _r = 0 ; _r < r ; _r++ ) {
      for ( int _c = 0 ; _c < c ; _c++ ) {
	color l = getpixel(_r, _c);
	l.r = (l.r - lowest.r) / mult.r;
	l.g = (l.g - lowest.g) / mult.g;
	l.b = (l.b - lowest.b) / mult.b;
	putpixel(_r, _c, l);
      }
    }
  }

  void save_to_ppm(string filename) {
    ofstream ofile(filename.c_str());

    const int max_color = 255;
    
    ofile << "P3" << endl;	    // [magic number]
    ofile << c << ' ' << r << endl; // [width] [height]
    ofile << max_color << endl;	    // [max value of colors]
    for ( int _r = 0 ; _r < r ; _r++ ) {
      for ( int _c = 0 ; _c < c ; _c++ ) {
        color l = getpixel(_r, _c);
	ofile
	  << (int)(l.r * max_color) << ' '
	  << (int)(l.g * max_color) << ' '
	  << (int)(l.b * max_color) << endl;
      }
    }
  }
};

void test_Image() {
  Image im(2, 3);
  im.
    putpixel(0, 0, color(1, 0, 0))->
    putpixel(0, 1, color(0, 1, 0))->
    putpixel(0, 2, color(0, 0, 1))->
    putpixel(1, 0, color(1, 1, 0))->
    putpixel(1, 1, color(1, 0, 1))->
    putpixel(1, 2, color(0, 1, 1))->
    save_to_ppm("x.ppm");
}

struct Vector {
  double x, y, z;
  Vector operator+ (const Vector& b) const {
    return Vector{x+b.x, y+b.y, z+b.z};
  }
  Vector operator- (const Vector& b) const {
    return Vector{x-b.x, y-b.y, z-b.z};
  }
  Vector operator- () const {
    return Vector{-x, -y, -z};
  }
  Vector operator* (const double &b) const {
    return Vector{x*b, y*b, z*b};
  }
  double dot(const Vector &b) const {
    return x*b.x + y*b.y + z*b.z;
  }
  double squared() const {
    return dot(*this);
  }
  double length() const {
    return sqrt(squared());
  }
  Vector unit() const {
    double r = length();
    return Vector{x/r, y/r, z/r};
  }
};

struct Ray {
  Vector pt, dir;
  color l;
};

typedef pair<bool, pair<Vector, color> > intersection;

class WorldObject {
public:
  virtual intersection intersect(Ray) const = 0;
  virtual ~WorldObject() {}
  // (is it intersecting, (where it intersected, color of
  // intersection))
};

inline double square(double a) {
  return a*a;
}

class Sphere : public WorldObject {
private:
  Vector centre;
  double r;
  color col;
public:
  Sphere(Vector pos, double radius, color l) :
    centre(pos), r(radius), col(l) {}
  intersection intersect(Ray ray) const {
    Vector c = centre;
    Vector s = ray.pt;
    Vector d = ray.dir;

    Vector v = s - c;

    double det = square(v.dot(d)) - (v.dot(v) - square(r));
    
    if ( det < 0 ) // can't have intersection
      return make_pair(false, make_pair(Vector(), color()));

    double dets = sqrt(det);
    double z = -(v.dot(d));
    double t1 = z + dets;
    double t2 = z - dets;

    double t = min(t1, t2);

    Vector y = s + d * t;	// point of intersection
    Vector n = (y - c).unit();	// normal surface

    double cosTheta = fabs(n.dot(d)); // for unit vectors, but n and d
				      // are unit

    color res{
      cosTheta * col.r * ray.l.r,
      cosTheta * col.g * ray.l.g,
      cosTheta * col.b * ray.l.b,
    };

    return make_pair(true, make_pair(y, res));
  }
};

vector<WorldObject*> world;

void initialize_world() {
  world.push_back(new Sphere{Vector{0, -300, 1200}, 200, color(1, 0, 0)});
  world.push_back(new Sphere{Vector{-80, -150, 1200}, 200, color(0, 1, 0)});
  world.push_back(new Sphere{Vector{70, -100, 1200}, 200, color(0, 0, 1)});

  for ( int z = 2 ; z <= 7 ; z++ ) {
    for ( int x = -2 ; x <= 2 ; x++ ) {
      world.push_back(new Sphere{
	  Vector{200. * x, 300, 400. * z}, 40, color(1, 1, 1)});
    }
  }
}

void cleanup_world() {
  int len = world.size();
  for ( int i = 0 ; i < len ; i++ ) {
    delete world[i];
  }
  world.clear();
}

/* Global constants */

const Vector eye { 0, 0, -200 };
const Vector screen_center {0,0,0};
const int width = 100, height = 100;
const int max_x = width / 2;
const int max_y = height / 2;

const int resolution = 10;

color shoot_ray(Vector from, Vector to) {
  Ray ray{from, (to-from).unit(), color{1,1,1}};

  bool hit_somewhere = false;
  Vector best_intersection{inf, inf, inf};
  color best_color;
  for ( vector<WorldObject*>::iterator it = world.begin() ;
	it != world.end() ;
	it++ ) {
    WorldObject* wo = *it;
    intersection i = wo->intersect(ray);
    bool did_intersect = i.first;
    Vector where_intersect = i.second.first;
    color color_intersect = i.second.second;
    
    if ( ! did_intersect ) // no intersection, ignore
      continue;

    hit_somewhere = true;
    if ( (where_intersect   - from).length() <
	 (best_intersection - from).length() ) {
      best_intersection = where_intersect;
      best_color = color_intersect;
    }
  }

  if ( ! hit_somewhere ) {
    best_color = color(0, 0, 0);
  }

  double dist_drop =
    square((screen_center     - eye ).length()) /
    square((best_intersection - from).length());
  
  return best_color * dist_drop;
}

int main() {
  initialize_world();
  Image im(height * resolution, width * resolution);
  for ( int r = 0 ; r < height * resolution ; r++ ) {
    for ( int c = 0 ; c < width * resolution ; c++ ) {
      double x = (double)c / resolution - (double)width / 2;
      double y = (double)r / resolution - (double)height / 2;
      double z = 0;
      Vector screen_point { x, y, z };
      im.putpixel(r, c, shoot_ray(eye, screen_point));
    }
  }
  im.normalize();
  im.save_to_ppm("x.ppm");
  cleanup_world();
}
