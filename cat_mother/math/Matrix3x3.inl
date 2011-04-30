inline void Matrix3x3::rotate( const Vector3& v, Vector3* v1 ) const
{
	float* d = &v1->x;
	d[0] = m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2];
	d[1] = m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2];
	d[2] = m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2];
}

inline Vector3 Matrix3x3::rotate( const Vector3& v ) const
{
	Vector3 v1;
	float* d = &v1.x;
	d[0] = m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2];
	d[1] = m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2];
	d[2] = m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2];
	return v1;
}
