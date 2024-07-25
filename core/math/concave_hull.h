/**************************************************************************/
/*  concave_hull.h                                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef CONCAVE_HULL_H
#define CONCAVE_HULL_H

//
// Adapted from:
// https://github.com/sadaszewski/concaveman-cpp
//

#include "core/math/vector2.h"
#include "core/templates/vector.h"
#include <array>
#include <list>
#include <memory>
#include <queue>
#include <vector>

template <class T>
class _CompareFirst {
public:
	bool operator()(const T &a, const T &b) {
		return (std::get<0>(a) < std::get<0>(b));
	}
};

real_t _orient2d(
		const Point2 &p1,
		const Point2 &p2,
		const Point2 &p3) {
	real_t res = (p2[1] - p1[1]) * (p3[0] - p2[0]) -
			(p2[0] - p1[0]) * (p3[1] - p2[1]);

	return res;
}

// check if the edges (p1,q1) and (p2,q2) intersect
bool _intersects(
		const Point2 &p1,
		const Point2 &q1,
		const Point2 &p2,
		const Point2 &q2) {
	bool res = (p1[0] != q2[0] || p1[1] != q2[1]) &&
			(q1[0] != p2[0] || q1[1] != p2[1]) &&
			(_orient2d(p1, q1, p2) > 0) != (_orient2d(p1, q1, q2) > 0) &&
			(_orient2d(p2, q2, p1) > 0) != (_orient2d(p2, q2, q1) > 0);

	return res;
}

// square distance between 2 points
real_t _get_sq_dist(
		const Point2 &p1,
		const Point2 &p2) {
	real_t dx = p1[0] - p2[0];
	real_t dy = p1[1] - p2[1];
	return dx * dx + dy * dy;
}

// square distance from a point to a segment
real_t _sq_seg_dist(
		const Point2 &p,
		const Point2 &p1,
		const Point2 &p2) {
	real_t x = p1[0];
	real_t y = p1[1];
	real_t dx = p2[0] - x;
	real_t dy = p2[1] - y;

	if (dx != 0 || dy != 0) {
		real_t t = ((p[0] - x) * dx + (p[1] - y) * dy) / (dx * dx + dy * dy);
		if (t > 1) {
			x = p2[0];
			y = p2[1];
		} else if (t > 0) {
			x += dx * t;
			y += dy * t;
		}
	}

	dx = p[0] - x;
	dy = p[1] - y;

	return dx * dx + dy * dy;
}

// segment to segment distance, ported from http://geomalgorithms.com/a07-_distance.html by Dan Sunday
real_t sq_seg_seg_dist(real_t x0, real_t y0,
		real_t x1, real_t y1,
		real_t x2, real_t y2,
		real_t x3, real_t y3) {
	real_t ux = x1 - x0;
	real_t uy = y1 - y0;
	real_t vx = x3 - x2;
	real_t vy = y3 - y2;
	real_t wx = x0 - x2;
	real_t wy = y0 - y2;
	real_t a = ux * ux + uy * uy;
	real_t b = ux * vx + uy * vy;
	real_t c = vx * vx + vy * vy;
	real_t d = ux * wx + uy * wy;
	real_t e = vx * wx + vy * wy;
	real_t D = a * c - b * b;

	real_t sc, sN, tc, tN;
	real_t sD = D;
	real_t tD = D;

	if (D == 0) {
		sN = 0;
		sD = 1;
		tN = e;
		tD = c;
	} else {
		sN = b * e - c * d;
		tN = a * e - b * d;
		if (sN < 0) {
			sN = 0;
			tN = e;
			tD = c;
		} else if (sN > sD) {
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if (tN < 0) {
		tN = 0;
		if (-d < 0) {
			sN = 0;
		} else if (-d > a) {
			sN = sD;
		} else {
			sN = -d;
			sD = a;
		}
	} else if (tN > tD) {
		tN = tD;
		if (-d + b < 0) {
			sN = 0;
		} else if (-d + b > a) {
			sN = sD;
		} else {
			sN = -d + b;
			sD = a;
		}
	}

	sc = ((sN == 0) ? 0 : sN / sD);
	tc = ((tN == 0) ? 0 : tN / tD);

	real_t cx = (1 - sc) * x0 + sc * x1;
	real_t cy = (1 - sc) * y0 + sc * y1;
	real_t cx2 = (1 - tc) * x2 + tc * x3;
	real_t cy2 = (1 - tc) * y2 + tc * y3;
	real_t dx = cx2 - cx;
	real_t dy = cy2 - cy;

	return dx * dx + dy * dy;
}

template <class DATA>
class _RTree {
public:
	constexpr static int DIM = 2;
	constexpr static int MAX_CHILDREN = 16;
	typedef std::array<real_t, DIM * 2> bounds_type;

	_RTree() :
			m_is_leaf(false), m_data() {
		for (int i = 0; i < DIM; i++) {
			m_bounds[i] = std::numeric_limits<real_t>::max();
			m_bounds[i + DIM] = std::numeric_limits<real_t>::min();
		}
	}

	_RTree(DATA data, const bounds_type &bounds) :
			m_is_leaf(true), m_data(data), m_bounds(bounds) {
		for (int i = 0; i < DIM; i++) {
			if (bounds[i] > bounds[i + DIM]) {
				ERR_FAIL_MSG("Bounds minima have to be less than maxima");
			}
		}
	}

	void insert(DATA data, const bounds_type &bounds) {
		if (m_is_leaf) {
			ERR_FAIL_MSG("Cannot insert into leaves");
		}
		m_bounds = updated_bounds(bounds);
		if (m_children.size() < MAX_CHILDREN) {
			std::unique_ptr<_RTree<DATA>> r = std::make_unique<_RTree<DATA>>(data, bounds);
			m_children.push_back(std::move(r));
			return;
		}

		std::reference_wrapper<_RTree<DATA>> best_child = *m_children.begin()->get();
		auto best_volume = volume(best_child.get().updated_bounds(bounds));
		for (auto it = ++m_children.begin(); it != m_children.end(); it++) {
			auto v = volume((*it)->updated_bounds(bounds));
			if (v < best_volume) {
				best_volume = v;
				best_child = *it->get();
			}
		}
		if (!best_child.get().is_leaf()) {
			best_child.get().insert(data, bounds);
			return;
		}

		auto leaf = std::make_unique<_RTree<DATA>>(best_child.get().data(),
				best_child.get().bounds());
		best_child.get().m_is_leaf = false;
		best_child.get().m_data = DATA();
		best_child.get().m_children.push_back(std::move(leaf));
		best_child.get().insert(data, bounds);
	}

	void intersection(const bounds_type &bounds,
			std::vector<std::reference_wrapper<const _RTree<DATA>>> res) const {
		if (!intersects(bounds)) {
			return;
		}
		if (m_is_leaf) {
			res.push_back(*this);
			return;
		}
		for (auto &ch : m_children) {
			ch->intersection(bounds, res);
		}
	}

	std::vector<std::reference_wrapper<const _RTree<DATA>>> intersection(const bounds_type &bounds) const {
		std::vector<std::reference_wrapper<const _RTree<DATA>>> res;
		intersection(bounds, res);
		return res;
	}

	bool intersects(const bounds_type &bounds) const {
		for (int i = 0; i < DIM; i++) {
			if (m_bounds[i] > bounds[i + DIM]) {
				return false;
			}
			if (m_bounds[i + DIM] < bounds[i]) {
				return false;
			}
		}
		return true;
	}

	void erase(DATA data, const bounds_type &bounds) {
		if (m_is_leaf) {
			ERR_FAIL_MSG("Cannot erase from leaves");
		}
		if (!intersects(bounds)) {
			return;
		}

		for (auto it = m_children.begin(); it != m_children.end();) {
			if (!(*it)->m_is_leaf) {
				(*it)->erase(data, bounds);
				it++;
			} else if ((*it)->m_data == data &&
					(*it)->m_bounds == bounds) {
				m_children.erase(it++);
			} else {
				it++;
			}
		}
	}

	bounds_type updated_bounds(const bounds_type &child_bounds) const {
		bounds_type res;
		for (int i = 0; i < DIM; i++) {
			res[i] = MIN(child_bounds[i], m_bounds[i]);
			res[i + DIM] = MAX(child_bounds[i + DIM], m_bounds[i + DIM]);
		}
		return res;
	}

	static real_t volume(const bounds_type &bounds) {
		real_t res = 1;
		for (int i = 0; i < DIM; i++) {
			real_t delta = bounds[i + DIM] - bounds[i];
			res *= delta;
		}
		return res;
	}

	const bounds_type &bounds() const {
		return m_bounds;
	}

	bool is_leaf() const {
		return m_is_leaf;
	}

	DATA data() const {
		return m_data;
	}

	const std::list<std::unique_ptr<_RTree<DATA>>> &children() const {
		return m_children;
	}

private:
	bool m_is_leaf;
	DATA m_data;
	std::list<std::unique_ptr<_RTree<DATA>>> m_children;
	bounds_type m_bounds;
};

struct _Node {
	_Node(const Point2 &p) {
		this->p = p;
	}

	Point2 p;
	real_t min_x = 0;
	real_t min_y = 0;
	real_t max_x = 0;
	real_t max_y = 0;
};

class _CircularList;

class _CircularElement {
public:
	template <class... Args>
	_CircularElement(Args &&...args) :
			m_data(std::forward<Args>(args)...) {
	}

	_Node &data() {
		return m_data;
	}

	template <class... Args>
	_CircularElement *insert(Args &&...args) {
		_CircularElement *elem = memnew(_CircularElement(std::forward<Args>(args)...));
		elem->m_prev = this;
		elem->m_next = m_next;
		m_next->m_prev = elem;
		m_next = elem;
		return elem;
	}

	_CircularElement *prev() {
		return m_prev;
	}

	_CircularElement *next() {
		return m_next;
	}

private:
	_Node m_data;
	_CircularElement *m_prev = nullptr;
	_CircularElement *m_next = nullptr;

	friend class _CircularList;
};

class _CircularList {
public:
	~_CircularList() {
		_CircularElement *node = m_last;
		while (true) {
			_CircularElement *tmp = node;
			node = node->m_next;
			memdelete(tmp);
			if (node == m_last) {
				break;
			}
		}
	}

	template <class... Args>
	_CircularElement *insert(_CircularElement *prev, Args &&...args) {
		_CircularElement *elem = memnew(_CircularElement(std::forward<Args>(args)...));

		if (prev == nullptr && m_last != nullptr) {
			ERR_FAIL_V_MSG(nullptr, "Once the list is non-empty you must specify where to insert");
		}

		if (prev == nullptr) {
			elem->m_prev = elem->m_next = elem;
		} else {
			elem->m_prev = prev;
			elem->m_next = prev->m_next;
			prev->m_next->m_prev = elem;
			prev->m_next = elem;
		}

		m_last = elem;

		return elem;
	}

private:
	_CircularElement *m_last = nullptr;
};

// update the bounding box of a node's edge
void _update_bbox(_CircularElement *elem) {
	_Node &node(elem->data());
	Point2 p1 = node.p;
	Point2 p2 = elem->next()->data().p;
	node.min_x = MIN(p1[0], p2[0]);
	node.min_y = MIN(p1[1], p2[1]);
	node.max_x = MAX(p1[0], p2[0]);
	node.max_y = MAX(p1[1], p2[1]);
}

// square distance from a segment bounding box to the given one
template <class DATA>
real_t _sq_seg_box_dist(
		const Point2 &a,
		const Point2 &b,
		const _RTree<DATA> &bbox) {
	typedef std::array<real_t, 4> bounds_type;

	if (inside(a, bbox) || inside(b, bbox)) {
		return 0;
	}

	const bounds_type &bounds = bbox.bounds();
	real_t min_x = bounds[0];
	real_t min_y = bounds[1];
	real_t max_x = bounds[2];
	real_t max_y = bounds[3];

	real_t d1 = sq_seg_seg_dist(a[0], a[1], b[0], b[1], min_x, min_y, max_x, min_y);
	if (d1 == 0) {
		return 0;
	}

	auto d2 = sq_seg_seg_dist(a[0], a[1], b[0], b[1], min_x, min_y, min_x, max_y);
	if (d2 == 0) {
		return 0;
	}

	auto d3 = sq_seg_seg_dist(a[0], a[1], b[0], b[1], max_x, min_y, max_x, max_y);
	if (d3 == 0) {
		return 0;
	}

	auto d4 = sq_seg_seg_dist(a[0], a[1], b[0], b[1], min_x, max_y, max_x, max_y);
	if (d4 == 0) {
		return 0;
	}

	return MIN(MIN(d1, d2), MIN(d3, d4));
}

// check if the edge (a,b) doesn't intersect any other edges
bool no_intersections(
		const Point2 &a,
		const Point2 &b,
		const _RTree<_CircularElement *> &segTree) {
	real_t min_x = MIN(a[0], b[0]);
	real_t min_y = MIN(a[1], b[1]);
	real_t max_x = MAX(a[0], b[0]);
	real_t max_y = MAX(a[1], b[1]);

	std::vector<std::reference_wrapper<const _RTree<_CircularElement *>>> isect = segTree.intersection({ min_x, min_y, max_x, max_y });

	for (const std::reference_wrapper<const _RTree<_CircularElement *>> &ch : isect) {
		_CircularElement *elem = ch.get().data();

		if (_intersects(elem->data().p, elem->next()->data().p, a, b)) {
			return false;
		}
	}

	return true;
}

Point2 _find_candidate(
		const _RTree<Point2> &tree,
		const Point2 &a,
		const Point2 &b,
		const Point2 &c,
		const Point2 &d,
		real_t maxDist,
		const _RTree<_CircularElement *> &segTree,
		bool &ok) {
	typedef std::tuple<real_t, std::reference_wrapper<const _RTree<Point2>>> tuple_type;
	typedef std::array<real_t, 4> bounds_type;
	ok = false;

	std::priority_queue<tuple_type, std::vector<tuple_type>, _CompareFirst<tuple_type>> queue;
	std::reference_wrapper<const _RTree<Point2>> node = tree;

	// search through the point R-tree with a depth-first search using a priority queue
	// in the order of distance to the edge (b, c)
	while (true) {
		for (const std::unique_ptr<_RTree<Vector2>> &child : node.get().children()) {
			bounds_type bounds = child->bounds();
			Point2 pt = { bounds[0], bounds[1] };

			real_t dist = child->is_leaf() ? _sq_seg_dist(pt, b, c) : _sq_seg_box_dist(b, c, *child);
			if (dist > maxDist) {
				continue; // skip the node if it's farther than we ever need
			}

			queue.push(tuple_type(-dist, *child));
		}

		while (!queue.empty() && std::get<1>(queue.top()).get().is_leaf()) {
			auto item = queue.top();
			queue.pop();

			bounds_type bounds = std::get<1>(item).get().bounds();
			Point2 p = { bounds[0], bounds[1] };

			// skip all points that are as close to adjacent edges (a,b) and (c,d),
			// and points that would introduce self-intersections when connected
			real_t d0 = _sq_seg_dist(p, a, b);
			real_t d1 = _sq_seg_dist(p, c, d);

			if (-std::get<0>(item) < d0 && -std::get<0>(item) < d1 &&
					no_intersections(b, p, segTree) &&
					no_intersections(c, p, segTree)) {
				ok = true;
				return std::get<1>(item).get().data();
			}
		}

		if (queue.empty()) {
			break;
		}

		node = std::get<1>(queue.top());
		queue.pop();
	}

	return Point2();
}

template <class DATA>
bool inside(
		const Point2 &a,
		const _RTree<DATA> &bbox) {
	auto &bounds = bbox.bounds();

	auto min_x = bounds[0];
	auto min_y = bounds[1];
	auto max_x = bounds[2];
	auto max_y = bounds[3];

	auto res = (a[0] >= min_x) &&
			(a[0] <= max_x) &&
			(a[1] >= min_y) &&
			(a[1] <= max_y);
	return res;
}

Vector<Point2> concaveman(
		const Vector<Point2> &points,
		// start with a convex hull of the points
		const Vector<Point2> &hull,
		// a relative measure of concavity; higher value means simpler hull
		real_t concavity = 2,
		// when a segment goes below this length threshold, it won't be drilled down further
		real_t lengthThreshold = 0) {
	// exit if hull includes all points already
	if (hull.size() == points.size()) {
		return hull;
	}

	// index the points with an R-tree
	_RTree<Point2> tree;

	for (const Point2 &p : points) {
		tree.insert(p, { p[0], p[1], p[0], p[1] });
	}

	_CircularList circList;
	_CircularElement *last = nullptr;

	std::list<_CircularElement *> queue;

	// turn the convex hull into a linked list and populate the initial edge queue with the nodes
	for (const Point2 &p : hull) {
		tree.erase(p, { p[0], p[1], p[0], p[1] });
		last = circList.insert(last, p);
		queue.push_back(last);
	}

	// index the segments with an R-tree (for intersection checks)
	_RTree<_CircularElement *> segTree;
	for (_CircularElement *elem : queue) {
		_Node &node(elem->data());
		_update_bbox(elem);
		segTree.insert(elem, { node.min_x, node.min_y, node.max_x, node.max_y });
	}

	real_t sqConcavity = concavity * concavity;
	real_t sqLenThreshold = lengthThreshold * lengthThreshold;

	// process edges one by one
	while (!queue.empty()) {
		_CircularElement *elem = *queue.begin();
		queue.pop_front();

		Point2 a = elem->prev()->data().p;
		Point2 b = elem->data().p;
		Point2 c = elem->next()->data().p;
		Point2 d = elem->next()->next()->data().p;

		// skip the edge if it's already short enough
		real_t sqLen = _get_sq_dist(b, c);
		if (sqLen < sqLenThreshold) {
			continue;
		}

		real_t maxSqLen = sqLen / sqConcavity;

		// find the best connection point for the current edge to flex inward to
		bool ok;
		Point2 p = _find_candidate(tree, a, b, c, d, maxSqLen, segTree, ok);

		// if we found a connection and it satisfies our concavity measure
		if (ok && MIN(_get_sq_dist(p, b), _get_sq_dist(p, c)) <= maxSqLen) {
			// connect the edge endpoints through this point and add 2 new edges to the queue
			queue.push_back(elem);
			queue.push_back(elem->insert(p));

			// update point and segment indexes
			_Node &node = elem->data();
			_Node &next = elem->next()->data();

			tree.erase(p, { p[0], p[1], p[0], p[1] });
			segTree.erase(elem, { node.min_x, node.min_y, node.max_x, node.max_y });

			_update_bbox(elem);
			_update_bbox(elem->next());

			segTree.insert(elem, { node.min_x, node.min_y, node.max_x, node.max_y });
			segTree.insert(elem->next(), { next.min_x, next.min_y, next.max_x, next.max_y });
		}
	}

	// convert the resulting hull linked list to an array of points
	Vector<Point2> concave;
	for (auto elem = last->next();; elem = elem->next()) {
		concave.push_back(Point2(elem->data().p[0], elem->data().p[1]));
		if (elem == last) {
			break;
		}
	}

	return concave;
}

#endif // CONCAVE_HULL_H
