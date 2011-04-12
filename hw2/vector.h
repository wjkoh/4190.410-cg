#ifndef _VECTOR_H_
#define _VECTOR_H_

namespace wjk
{
template<class T, int DIM>
class vector
{
    T elems[DIM];

    public:
    T& x;
    T& y;
    T& z;

    vector() : x(elems[0]) , y(elems[1]) , z(elems[2]) {

        elems[0] = 0;
        elems[1] = 0;
        elems[2] = 0;
    }
    vector(const T& x, const T& y, const T& z)
        : x(elems[0]) , y(elems[1]) , z(elems[2])
    {
        elems[0] = x;
        elems[1] = y;
        elems[2] = z;
    }

    vector(const vector<T, DIM>& rhs)
        : x(elems[0]) , y(elems[1]) , z(elems[2])
    {
        for (int i = 0; i < DIM; ++i)
            elems[i] = rhs.elems[i];
    }

    // get
    T& operator[](int index) { return this->elems[index]; }
    const T& operator[](int index) const { return this->elems[index]; }

    // set
    void set(const T& x, const T& y, const T& z)
    {
        elems[0] = x;
        elems[1] = y;
        elems[2] = z;
    }

    vector<T, DIM>& operator=(const vector<T, DIM>& rhs)
    {
        if (this != &rhs) 
        {
            for (int i = 0; i < DIM; ++i)
                elems[i] = rhs.elems[i];
        }
        return *this;
    }

    vector<T, DIM> operator+(const vector<T, DIM>& rhs) const
    {
        return vector<T, DIM>(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z);
    }

    vector<T, DIM> operator-(const vector<T, DIM>& rhs) const
    {
        return vector<T, DIM>(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z);
    }

    bool operator<(const vector<T, DIM>& rhs) const
    {
        return (this->sq_sum() < rhs.sq_sum());
    }

    // scalar product
    template <class U>
    vector<T, DIM> operator*(U rhs)
    {
        vector<T, DIM> result(*this);
        for (int i = 0; i < DIM; ++i)
            result.elems[i] *= rhs;
        return result;
    }

    // inner product
    T operator*(const vector<T, DIM>& rhs)
    {
        return (this->x*rhs.x + this->y*rhs.y + this->z*rhs.z);
    }

    // cross product
    vector<T, 3> operator/(const vector<T, 3>& rhs)
    {
        vector<T, 3> result;
        result.x = (this->y*rhs.z - this->z*rhs.y);
        result.y = (this->z*rhs.x - this->x*rhs.z);
        result.z = (this->x*rhs.y - this->y*rhs.x);
        return result;
    }

    long double sq_sum() const
    {
        long double sq_sum = 0;
        for (int i = 0; i < DIM; ++i)
            sq_sum += pow(elems[i], 2);
        return sq_sum;
    }

    void normalize()
    {
        long double sq_sum = this->sq_sum();

        // divide by zero
        if (sq_sum == 0) return;

        long double sqrt_sq_sum = sqrt(sq_sum);
        for (int i = 0; i < DIM; ++i)
            elems[i] /= sqrt_sq_sum;
    }
};

template<class T, int DIM>
ostream& operator<<(ostream& os, const vector<T, DIM>& rhs)
{
    os << "V ";
    os << "(";
    for (int i = 0; i < DIM; ++i)
    {
        if (i != 0) os << ", ";
        os << rhs[i];
    }
    os << ")";
    return os;
}
}

// typedefs
typedef wjk::vector<float, 3> vector3f;
typedef wjk::vector<double, 3> vector3d;

#endif // _VECTOR_H_
