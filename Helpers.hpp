#pragma once
#include <SFML/Graphics.hpp>

struct CompareVector2i
{
	bool operator()(sf::Vector2i a, sf::Vector2i b) const
	{
		if (a.x < b.x)
			return true;
		else if (b.x < a.x)
			return false;
		else
			return a.y < b.y;
	}
};

// sf::Vector2 helpers
template <typename T>
std::ostream & operator<<(std::ostream & os, const sf::Vector2<T>& v)
{
	os << v.x << "x" << v.y;
	return os;
}

template <typename T>
sf::Vector2<T> operator *(const sf::Vector2<T>& left, const sf::Vector2<T>& right)
{
	T x = left.x * right.x;
	T y = left.y * right.y;
	return sf::Vector2<T>(x, y);
}

template <typename T>
sf::Vector2<T> operator /(const sf::Vector2<T>& left, const sf::Vector2<T>& right)
{
	T x = left.x / right.x;
	T y = left.y / right.y;
	return sf::Vector2<T>(x, y);
}

template <typename T>
sf::Vector2<T> operator +(const sf::Vector2<T>& v, const T &sum)
{
	T x = v.x + sum;
	T y = v.y + sum;
	return sf::Vector2<T>(x, y);
}

template <typename T>
sf::Vector2<T> operator -(const sf::Vector2<T>& v, const T &sum)
{
	T x = v.x - sum;
	T y = v.y - sum;
	return sf::Vector2<T>(x, y);
}

template <typename T>
float square(const sf::Vector2<T> &v) {
	return (v.x * v.x + v.y * v.y);
}

//#define APPROXIMATE_DISTANCE

template <typename T>
float distance(const sf::Vector2<T> &p1, const sf::Vector2<T> &p2) {
#ifdef APPROXIMATE_DISTANCE
	float dx = abs(p1.x - p2.x);
	float dy = abs(p1.y - p2.y);
	return 0.394f * (dx + dy) + 0.554f * std::max(dx, dy);
#else
	return sqrt((float)(p2.x - p1.x) * (float)(p2.x - p1.x) + (float)(p2.y - p1.y) * (float)(p2.y - p1.y));
#endif
}

template <typename T>
float length(const sf::Vector2<T> &v) {
	return sqrt(v.x * v.x + v.y * v.y);
}

template <typename T>
sf::Vector2<T> normalize(const sf::Vector2<T> &v) {
	sf::Vector2<T> nv;
	float len = length(v);
	if (len == 0) {
		nv.y = 0; nv.x = 0;
		len = 1;
	}
	nv.x = v.x / len;
	nv.y = v.y / len;
	return nv;
}

template <typename T>
sf::Vector2<T> round(const sf::Vector2<T> &v) {
	sf::Vector2<T> rv = v;
	rv.x = round(rv.x);
	rv.y = round(rv.y);
	return rv;
}

template <typename T>
sf::Vector2<T> trunc(const sf::Vector2<T> &v) {
	sf::Vector2<T> rv = v;
	rv.x = trunc(rv.x);
	rv.y = trunc(rv.y);
	return rv;
}

template <typename T>
sf::Vector2<T> limit(const sf::Vector2<T> &v, const T &max) {
	if (square(v) > max * max) {
		return normalize(v) * max;
	} else {
		return v;
	}
}


template <typename T>
sf::Vector2<T> limit(const sf::Vector2<T> &v, const T &min, const T &max) {
	if (square(v) > max * max) {
		return normalize(v) * max;
	} else {
		if (square(v) < min) {
			return sf::Vector2<T>(0, 0);
		} else {
			return v;
		}
	}
}
