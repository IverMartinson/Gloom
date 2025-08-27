#include <cmath>

struct vector2{
    double x;
    double y;

    vector2(double x = 0, double y = 0) : x(x), y(y) {}

    vector2 operator+(const vector2& other) const {
        return vector2(x + other.x, y + other.y);
    }
    
    vector2 operator-(const vector2& other) const {
        return vector2(x - other.x, y - other.y);
    }
    
    vector2 operator*(const double& other) const {
        return vector2(x * other, y * other);
    }
    
    vector2 operator/(const double& other) const {
        return vector2(x / other, y / other);
    }
    
    double magnitude (){
    	return sqrt(x*x + y*y);
    }
    
};

struct vector3{
    double x;
    double y;
    double z;

    vector3(double x=0, double y=0, double z=0) : x(x), y(y), z(z) {}

    vector3 operator+(const vector3& other) const {
        return vector3(x + other.x, y + other.y, z + other.z);
    }    
    
    vector3 operator-(const vector3& other) const {
        return vector3(x - other.x, y - other.y, z - other.z);
    }
    
    vector3 operator*(const double& other) const {
        return vector3(x * other, y * other, z * other);
    }
    
    vector3 operator/(const double& other) const {
        return vector3(x / other, y / other, z / other);
    }
    
    double magnitude (){
    	return sqrt(x*x + y*y + z*z);
    }
};

double distance_2d(double x1, double y1, double x2, double y2){
    return sqrt(pow(x1-x2, 2) + pow(y1-y2, 2));
}

double distance_2d_v(vector2 a, vector2 b) {
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}


