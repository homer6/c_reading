inline void Matrix4x4::transform( const Vector3& v, Vector3* v1 ) const
{
	float* d = &v1->x;
	d[0] = m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2] + m[0][3];
	d[1] = m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2] + m[1][3];
	d[2] = m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2] + m[2][3];
}

inline void Matrix4x4::transform( const Vector4& v, Vector4* v1 ) const
{
	float* d = &v1->x;
	d[0] = m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2] + m[0][3]*v[3];
	d[1] = m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2] + m[1][3]*v[3];
	d[2] = m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2] + m[2][3]*v[3];
	d[3] = m[3][0]*v[0] + m[3][1]*v[1] + m[3][2]*v[2] + m[3][3]*v[3];
}

inline void Matrix4x4::rotate( const Vector3& v, Vector3* v1 ) const
{
	float* d = &v1->x;
	d[0] = m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2];
	d[1] = m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2];
	d[2] = m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2];
}
