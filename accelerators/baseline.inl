namespace fcpw {

template<int DIM>
inline Baseline<DIM>::Baseline(const std::vector<std::shared_ptr<Primitive<DIM>>>& primitives_):
primitives(primitives_)
{

}

template<int DIM>
inline BoundingBox<DIM> Baseline<DIM>::boundingBox() const
{
	BoundingBox<DIM> bb;
	for (int p = 0; p < (int)primitives.size(); p++) {
		bb.expandToInclude(primitives[p]->boundingBox());
	}

	return bb;
}

template<int DIM>
inline Vector<DIM> Baseline<DIM>::centroid() const
{
	Vector<DIM> c = zeroVector<DIM>();
	int nPrimitives = (int)primitives.size();

	for (int p = 0; p < nPrimitives; p++) {
		c += primitives[p]->centroid();
	}

	return c/nPrimitives;
}

template<int DIM>
inline float Baseline<DIM>::surfaceArea() const
{
	float area = 0.0f;
	for (int p = 0; p < (int)primitives.size(); p++) {
		area += primitives[p]->surfaceArea();
	}

	return area;
}

template<int DIM>
inline float Baseline<DIM>::signedVolume() const
{
	float volume = 0.0f;
	for (int p = 0; p < (int)primitives.size(); p++) {
		volume += primitives[p]->signedVolume();
	}

	return volume;
}

template<int DIM>
inline int Baseline<DIM>::intersectFromNode(Ray<DIM>& r, std::vector<Interaction<DIM>>& is,
											int nodeStartIndex, int& nodesVisited,
											bool checkOcclusion, bool countHits) const
{
#ifdef PROFILE
	PROFILE_SCOPED();
#endif

	int hits = 0;
	if (!countHits) is.resize(1);

	for (int p = 0; p < (int)primitives.size(); p++) {
		if (this->ignorePrimitive(primitives[p].get())) continue;

		std::vector<Interaction<DIM>> cs;
		int hit = primitives[p]->intersect(r, cs, checkOcclusion, countHits);
		nodesVisited++;

		if (hit > 0) {
			hits += hit;
			if (countHits) {
				is.insert(is.end(), cs.begin(), cs.end());

			} else {
				r.tMax = std::min(r.tMax, cs[0].d);
				is[0] = cs[0];
			}

			if (checkOcclusion) return 1;
		}
	}

	if (countHits) {
		std::sort(is.begin(), is.end(), compareInteractions<DIM>);
		is = removeDuplicates<DIM>(is);
		hits = (int)is.size();
	}

	return hits;
}

template<int DIM>
inline bool Baseline<DIM>::findClosestPointFromNode(BoundingSphere<DIM>& s, Interaction<DIM>& i,
													int nodeStartIndex, int& nodesVisited) const
{
#ifdef PROFILE
	PROFILE_SCOPED();
#endif

	bool notFound = true;
	for (int p = 0; p < (int)primitives.size(); p++) {
		if (this->ignorePrimitive(primitives[p].get())) continue;

		Interaction<DIM> c;
		bool found = primitives[p]->findClosestPoint(s, c);
		nodesVisited++;

		// keep the closest point only
		if (found) {
			notFound = false;
			s.r2 = std::min(s.r2, c.d*c.d);
			i = c;
		}
	}

	return !notFound;
}

} // namespace fcpw
