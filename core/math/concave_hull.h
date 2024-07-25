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
// Author: Stanislaw Adaszewski, 2019
// C++ port from https://github.com/mapbox/concaveman (js)

#include "core/math/vector2.h"
#include "core/templates/vector.h"
#include <limits>
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

template <class T, int DIM, int MAX_CHILDREN, class DATA>
class _RTree {
public:
	typedef _RTree<T, DIM, MAX_CHILDREN, DATA> type;
	typedef const type const_type;
	typedef type *type_ptr;
	typedef const type *type_const_ptr;
	typedef std::array<T, DIM * 2> bounds_type;
	typedef DATA data_type;

	_RTree() :
			m_is_leaf(false), m_data() {
		for (int i = 0; i < DIM; i++) {
			m_bounds[i] = std::numeric_limits<T>::max();
			m_bounds[i + DIM] = std::numeric_limits<T>::min();
		}
	}

	_RTree(data_type data, const bounds_type &bounds) :
			m_is_leaf(true), m_data(data), m_bounds(bounds) {
		for (int i = 0; i < DIM; i++) {
			if (bounds[i] > bounds[i + DIM]) {
				ERR_FAIL_MSG("Bounds minima have to be less than maxima");
			}
		}
	}

	void insert(data_type data, const bounds_type &bounds) {
		if (m_is_leaf) {
			ERR_FAIL_MSG("Cannot insert into leaves");
		}
		m_bounds = updated_bounds(bounds);
		if (m_children.size() < MAX_CHILDREN) {
			auto r = std::make_unique<type>(data, bounds);
			m_children.push_back(std::move(r));
			return;
		}

		std::reference_wrapper<type> best_child = *m_children.begin()->get();
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

		auto leaf = std::make_unique<type>(best_child.get().data(),
				best_child.get().bounds());
		best_child.get().m_is_leaf = false;
		best_child.get().m_data = data_type();
		best_child.get().m_children.push_back(std::move(leaf));
		best_child.get().insert(data, bounds);
	}

	void intersection(const bounds_type &bounds,
			std::vector<std::reference_wrapper<const_type>> &res) const {
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

	std::vector<std::reference_wrapper<const_type>> intersection(const bounds_type &bounds) const {
		std::vector<std::reference_wrapper<const_type>> res;
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

	void erase(data_type data, const bounds_type &bounds) {
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
		for (auto i = 0; i < DIM; i++) {
			res[i] = MIN(child_bounds[i], m_bounds[i]);
			res[i + DIM] = MAX(child_bounds[i + DIM], m_bounds[i + DIM]);
		}
		return res;
	}

	static T volume(const bounds_type &bounds) {
		T res = 1;
		for (auto i = 0; i < DIM; i++) {
			auto delta = bounds[i + DIM] - bounds[i];
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

	data_type data() const {
		return m_data;
	}

	const std::list<std::unique_ptr<type>> &children() const {
		return m_children;
	}

private:
	bool m_is_leaf;
	data_type m_data;
	std::list<std::unique_ptr<type>> m_children;
	bounds_type m_bounds;
};

template <class T>
struct _Node {
	typedef _Node<T> type;
	typedef type *type_ptr;
	typedef Point2 point_type;

	_Node() :
			minX(),
			minY(),
			maxX(),
			maxY() {
	}

	_Node(const point_type &p) :
			_Node() {
		this->p = p;
	}

	point_type p;
	T minX;
	T minY;
	T maxX;
	T maxY;
};

template <class T>
class _CircularList;

template <class T>
class _CircularElement {
public:
	typedef _CircularElement<T> type;
	typedef type *ptr_type;

	template <class... Args>
	_CircularElement<T>(Args &&...args) :
			m_data(std::forward<Args>(args)...) {
	}

	T &data() {
		return m_data;
	}

	template <class... Args>
	_CircularElement<T> *insert(Args &&...args) {
		auto elem = new _CircularElement<T>(std::forward<Args>(args)...);
		elem->m_prev = this;
		elem->m_next = m_next;
		m_next->m_prev = elem;
		m_next = elem;
		return elem;
	}

	_CircularElement<T> *prev() {
		return m_prev;
	}

	_CircularElement<T> *next() {
		return m_next;
	}

private:
	T m_data;
	_CircularElement<T> *m_prev;
	_CircularElement<T> *m_next;

	friend class _CircularList<T>;
};

template <class T>
class _CircularList {
public:
	typedef _CircularElement<T> element_type;

	_CircularList() :
			m_last(nullptr) {
	}

	~_CircularList() {
		auto node = m_last;
		while (true) {
			auto tmp = node;
			node = node->m_next;
			delete tmp;
			if (node == m_last) {
				break;
			}
		}
	}

	template <class... Args>
	_CircularElement<T> *insert(element_type *prev, Args &&...args) {
		auto elem = new _CircularElement<T>(std::forward<Args>(args)...);

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
	element_type *m_last;
};

// update the bounding box of a node's edge
template <class T>
void _update_bbox(typename _CircularElement<T>::ptr_type elem) {
	auto &node(elem->data());
	auto p1 = node.p;
	auto p2 = elem->next()->data().p;
	node.minX = MIN(p1[0], p2[0]);
	node.minY = MIN(p1[1], p2[1]);
	node.maxX = MAX(p1[0], p2[0]);
	node.maxY = MAX(p1[1], p2[1]);
}

template <class T, int MAX_CHILDREN>
Point2 _find_candidate(
		const _RTree<T, 2, MAX_CHILDREN, Point2> &tree,
		const Point2 &a,
		const Point2 &b,
		const Point2 &c,
		const Point2 &d,
		T maxDist,
		const _RTree<T, 2, MAX_CHILDREN, typename _CircularElement<_Node<T>>::ptr_type> &segTree,
		bool &ok) {
	typedef Point2 point_type;
	typedef _RTree<T, 2, MAX_CHILDREN, Point2> tree_type;
	typedef const tree_type const_tree_type;
	typedef std::reference_wrapper<const_tree_type> tree_ref_type;
	typedef std::tuple<T, tree_ref_type> tuple_type;

	ok = false;

	std::priority_queue<tuple_type, std::vector<tuple_type>, _CompareFirst<tuple_type>> queue;
	std::reference_wrapper<const_tree_type> node = tree;

	// search through the point R-tree with a depth-first search using a priority queue
	// in the order of distance to the edge (b, c)
	while (true) {
		for (auto &child : node.get().children()) {
			auto bounds = child->bounds();
			point_type pt = { bounds[0], bounds[1] };

			auto dist = child->is_leaf() ? _sq_seg_dist(pt, b, c) : _sq_seg_box_dist(b, c, *child);
			if (dist > maxDist) {
				continue; // skip the node if it's farther than we ever need
			}

			queue.push(tuple_type(-dist, *child));
		}

		while (!queue.empty() && std::get<1>(queue.top()).get().is_leaf()) {
			auto item = queue.top();
			queue.pop();

			auto bounds = std::get<1>(item).get().bounds();
			point_type p = { bounds[0], bounds[1] };

			// skip all points that are as close to adjacent edges (a,b) and (c,d),
			// and points that would introduce self-intersections when connected
			auto d0 = _sq_seg_dist(p, a, b);
			auto d1 = _sq_seg_dist(p, c, d);

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

	return point_type();
}

// square distance from a segment bounding box to the given one
template <class T, int MAX_CHILDREN, class USER_DATA>
T _sq_seg_box_dist(
		const Point2 &a,
		const Point2 &b,
		const _RTree<T, 2, MAX_CHILDREN, USER_DATA> &bbox) {
	if (inside(a, bbox) || inside(b, bbox)) {
		return 0;
	}

	auto &bounds = bbox.bounds();
	auto minX = bounds[0];
	auto minY = bounds[1];
	auto maxX = bounds[2];
	auto maxY = bounds[3];

	auto d1 = sq_seg_seg_dist(a[0], a[1], b[0], b[1], minX, minY, maxX, minY);
	if (d1 == 0) {
		return 0;
	}

	auto d2 = sq_seg_seg_dist(a[0], a[1], b[0], b[1], minX, minY, minX, maxY);
	if (d2 == 0) {
		return 0;
	}

	auto d3 = sq_seg_seg_dist(a[0], a[1], b[0], b[1], maxX, minY, maxX, maxY);
	if (d3 == 0) {
		return 0;
	}

	auto d4 = sq_seg_seg_dist(a[0], a[1], b[0], b[1], minX, maxY, maxX, maxY);
	if (d4 == 0) {
		return 0;
	}

	return MIN(MIN(d1, d2), MIN(d3, d4));
}

template <class T, int MAX_CHILDREN, class USER_DATA>
bool inside(
		const Point2 &a,
		const _RTree<T, 2, MAX_CHILDREN, USER_DATA> &bbox) {
	auto &bounds = bbox.bounds();

	auto minX = bounds[0];
	auto minY = bounds[1];
	auto maxX = bounds[2];
	auto maxY = bounds[3];

	auto res = (a[0] >= minX) &&
			(a[0] <= maxX) &&
			(a[1] >= minY) &&
			(a[1] <= maxY);
	return res;
}

// check if the edge (a,b) doesn't intersect any other edges
template <class T, int MAX_CHILDREN>
bool no_intersections(
		const Point2 &a,
		const Point2 &b,
		const _RTree<T, 2, MAX_CHILDREN, typename _CircularElement<_Node<T>>::ptr_type> &segTree) {
	auto minX = MIN(a[0], b[0]);
	auto minY = MIN(a[1], b[1]);
	auto maxX = MAX(a[0], b[0]);
	auto maxY = MAX(a[1], b[1]);

	auto isect = segTree.intersection({ minX, minY, maxX, maxY });

	for (decltype(segTree) &ch : isect) {
		auto elem = ch.data();

		if (_intersects(elem->data().p, elem->next()->data().p, a, b)) {
			return false;
		}
	}

	return true;
}

template <class T, int MAX_CHILDREN>
Vector<Point2> concaveman(
		const Vector<Point2> &points,
		// start with a convex hull of the points
		const Vector<Point2> &hull,
		// a relative measure of concavity; higher value means simpler hull
		T concavity = 2,
		// when a segment goes below this length threshold, it won't be drilled down further
		T lengthThreshold = 0) {
	typedef _Node<T> node_type;
	typedef Point2 point_type;
	typedef _CircularElement<node_type> circ_elem_type;
	typedef _CircularList<node_type> circ_list_type;
	typedef circ_elem_type *circ_elem_ptr_type;

	// exit if hull includes all points already
	if (hull.size() == points.size()) {
		return hull;
	}

	// std::vector<std::array<real_t, 2>> points;
	// for (int i = 0; i < p_points.size(); i++) {
	// 	points.push_back({ p_points[i].x, p_points[i].y });
	// }
	// std::vector<std::array<real_t, 2>> hull;
	// for (int i = 0; i < p_hull.size(); i++) {
	// 	hull.push_back({ p_hull[i].x, p_hull[i].y });
	// }

	// index the points with an R-tree
	_RTree<T, 2, MAX_CHILDREN, point_type> tree;

	for (auto &p : points) {
		tree.insert(p, { p[0], p[1], p[0], p[1] });
	}

	circ_list_type circList;
	circ_elem_ptr_type last = nullptr;

	std::list<circ_elem_ptr_type> queue;

	// turn the convex hull into a linked list and populate the initial edge queue with the nodes
	for (auto &p : hull) {
		tree.erase(p, { p[0], p[1], p[0], p[1] });
		last = circList.insert(last, p);
		queue.push_back(last);
	}

	// index the segments with an R-tree (for intersection checks)
	_RTree<T, 2, MAX_CHILDREN, circ_elem_ptr_type> segTree;
	for (auto &elem : queue) {
		auto &node(elem->data());
		_update_bbox<node_type>(elem);
		segTree.insert(elem, { node.minX, node.minY, node.maxX, node.maxY });
	}

	auto sqConcavity = concavity * concavity;
	auto sqLenThreshold = lengthThreshold * lengthThreshold;

	// process edges one by one
	while (!queue.empty()) {
		auto elem = *queue.begin();
		queue.pop_front();

		auto a = elem->prev()->data().p;
		auto b = elem->data().p;
		auto c = elem->next()->data().p;
		auto d = elem->next()->next()->data().p;

		// skip the edge if it's already short enough
		auto sqLen = _get_sq_dist(b, c);
		if (sqLen < sqLenThreshold) {
			continue;
		}

		auto maxSqLen = sqLen / sqConcavity;

		// find the best connection point for the current edge to flex inward to
		bool ok;
		auto p = _find_candidate(tree, a, b, c, d, maxSqLen, segTree, ok);

		// if we found a connection and it satisfies our concavity measure
		if (ok && MIN(_get_sq_dist(p, b), _get_sq_dist(p, c)) <= maxSqLen) {
			// connect the edge endpoints through this point and add 2 new edges to the queue
			queue.push_back(elem);
			queue.push_back(elem->insert(p));

			// update point and segment indexes
			auto &node = elem->data();
			auto &next = elem->next()->data();

			tree.erase(p, { p[0], p[1], p[0], p[1] });
			segTree.erase(elem, { node.minX, node.minY, node.maxX, node.maxY });

			_update_bbox<node_type>(elem);
			_update_bbox<node_type>(elem->next());

			segTree.insert(elem, { node.minX, node.minY, node.maxX, node.maxY });
			segTree.insert(elem->next(), { next.minX, next.minY, next.maxX, next.maxY });
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
