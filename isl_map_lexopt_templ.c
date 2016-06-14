/*
 * Copyright 2010      INRIA Saclay
 * Copyright 2012      Ecole Normale Superieure
 *
 * Use of this software is governed by the MIT license
 *
 * Written by Sven Verdoolaege,
 * INRIA Saclay - Ile-de-France, Parc Club Orsay Universite,
 * ZAC des vignes, 4 rue Jacques Monod, 91893 Orsay, France
 * and Ecole Normale Superieure, 45 rue d’Ulm, 75230 Paris, France
 */

/* Function for computing the lexicographic optimum of a map
 * in the form of either an isl_map or an isl_pw_multi_aff.
 */

#define xSF(TYPE,SUFFIX) TYPE ## SUFFIX
#define SF(TYPE,SUFFIX) xSF(TYPE,SUFFIX)

/* Compute the lexicographic minimum (or maximum if "flags" includes
 * ISL_OPT_MAX) of "bmap" over the domain "dom" and return the result.
 * If "empty" is not NULL, then *empty is assigned a set that
 * contains those parts of the domain where there is no solution.
 * If "bmap" is marked as rational (ISL_BASIC_MAP_RATIONAL),
 * then the rational optimum is computed.  Otherwise, the integral optimum
 * is computed.
 */
static __isl_give TYPE *SF(isl_basic_map_partial_lexopt,SUFFIX)(
	__isl_take isl_basic_map *bmap, __isl_take isl_basic_set *dom,
	__isl_give isl_set **empty, unsigned flags)
{
	return SF(isl_tab_basic_map_partial_lexopt,SUFFIX)(bmap, dom, empty,
							    flags);
}

__isl_give TYPE *SF(isl_basic_map_partial_lexmax,SUFFIX)(
	__isl_take isl_basic_map *bmap, __isl_take isl_basic_set *dom,
	__isl_give isl_set **empty)
{
	unsigned flags = ISL_OPT_MAX;
	return SF(isl_basic_map_partial_lexopt,SUFFIX)(bmap, dom, empty, flags);
}

__isl_give TYPE *SF(isl_basic_map_partial_lexmin,SUFFIX)(
	__isl_take isl_basic_map *bmap, __isl_take isl_basic_set *dom,
	__isl_give isl_set **empty)
{
	unsigned flags = 0;
	return SF(isl_basic_map_partial_lexopt,SUFFIX)(bmap, dom, empty, flags);
}

__isl_give TYPE *SF(isl_basic_set_partial_lexmin,SUFFIX)(
	__isl_take isl_basic_set *bset, __isl_take isl_basic_set *dom,
	__isl_give isl_set **empty)
{
	return SF(isl_basic_map_partial_lexmin,SUFFIX)(bset, dom, empty);
}

__isl_give TYPE *SF(isl_basic_set_partial_lexmax,SUFFIX)(
	__isl_take isl_basic_set *bset, __isl_take isl_basic_set *dom,
	__isl_give isl_set **empty)
{
	return SF(isl_basic_map_partial_lexmax,SUFFIX)(bset, dom, empty);
}

/* Given a basic map "bmap", compute the lexicographically minimal
 * (or maximal) image element for each domain element in dom.
 * If empty is not NULL, then set *empty to those elements in dom
 * that do not have an image element.
 *
 * We first make sure the basic sets in dom are disjoint and then
 * simply collect the results over each of the basic sets separately.
 * We could probably improve the efficiency a bit by moving the union
 * domain down into the parametric integer programming.
 */
static __isl_give TYPE *SF(basic_map_partial_lexopt,SUFFIX)(
	__isl_take isl_basic_map *bmap, __isl_take isl_set *dom,
	__isl_give isl_set **empty, unsigned flags)
{
	int i;
	TYPE *res;
	isl_set *all_empty;

	dom = isl_set_make_disjoint(dom);
	if (!dom)
		goto error;

	if (isl_set_plain_is_empty(dom)) {
		isl_space *space = isl_basic_map_get_space(bmap);
		if (empty)
			*empty = dom;
		else
			isl_set_free(dom);
		isl_basic_map_free(bmap);
		return EMPTY(space);
	}

	res = SF(isl_basic_map_partial_lexopt,SUFFIX)(isl_basic_map_copy(bmap),
			isl_basic_set_copy(dom->p[0]), empty, flags);

	if (empty)
		all_empty = *empty;
	for (i = 1; i < dom->n; ++i) {
		TYPE *res_i;

		res_i = SF(isl_basic_map_partial_lexopt,SUFFIX)(
				isl_basic_map_copy(bmap),
				isl_basic_set_copy(dom->p[i]), empty, flags);

		res = ADD(res, res_i);
		if (empty)
			all_empty = isl_set_union_disjoint(all_empty, *empty);
	}

	if (empty)
		*empty = all_empty;
	isl_set_free(dom);
	isl_basic_map_free(bmap);
	return res;
error:
	if (empty)
		*empty = NULL;
	isl_set_free(dom);
	isl_basic_map_free(bmap);
	return NULL;
}

/* Compute the lexicographic minimum (or maximum if "flags" includes
 * ISL_OPT_MAX) of "bmap" over its domain.
 */
__isl_give TYPE *SF(isl_basic_map_lexopt,SUFFIX)(
	__isl_take isl_basic_map *bmap, unsigned flags)
{
	isl_basic_set *dom;

	dom = extract_domain(bmap);
	return SF(isl_basic_map_partial_lexopt,SUFFIX)(bmap, dom, NULL, flags);
}

__isl_give TYPE *SF(isl_basic_map_lexmin,SUFFIX)(__isl_take isl_basic_map *bmap)
{
	return SF(isl_basic_map_lexopt,SUFFIX)(bmap, 0);
}

static __isl_give TYPE *SF(isl_map_partial_lexopt_aligned,SUFFIX)(
	__isl_take isl_map *map, __isl_take isl_set *dom,
	__isl_give isl_set **empty, unsigned flags);

/* Given a map "map", compute the lexicographically minimal
 * (or maximal) image element for each domain element in dom.
 * Set *empty to those elements in dom that do not have an image element.
 *
 * Align parameters if needed and then call isl_map_partial_lexopt_aligned.
 */
static __isl_give TYPE *SF(isl_map_partial_lexopt,SUFFIX)(
	__isl_take isl_map *map, __isl_take isl_set *dom,
	__isl_give isl_set **empty, unsigned flags)
{
	if (!map || !dom)
		goto error;
	if (isl_space_match(map->dim, isl_dim_param, dom->dim, isl_dim_param))
		return SF(isl_map_partial_lexopt_aligned,SUFFIX)(map, dom,
								empty, flags);
	if (!isl_space_has_named_params(map->dim) ||
	    !isl_space_has_named_params(dom->dim))
		isl_die(map->ctx, isl_error_invalid,
			"unaligned unnamed parameters", goto error);
	map = isl_map_align_params(map, isl_map_get_space(dom));
	dom = isl_map_align_params(dom, isl_map_get_space(map));
	return SF(isl_map_partial_lexopt_aligned,SUFFIX)(map, dom, empty,
							flags);
error:
	if (empty)
		*empty = NULL;
	isl_set_free(dom);
	isl_map_free(map);
	return NULL;
}

__isl_give TYPE *SF(isl_map_lexopt,SUFFIX)(__isl_take isl_map *map,
	unsigned flags)
{
	isl_set *dom = NULL;
	isl_space *dom_space;

	if (!map)
		goto error;
	dom_space = isl_space_domain(isl_space_copy(map->dim));
	dom = isl_set_universe(dom_space);
	return SF(isl_map_partial_lexopt,SUFFIX)(map, dom, NULL, flags);
error:
	isl_map_free(map);
	return NULL;
}

__isl_give TYPE *SF(isl_map_lexmin,SUFFIX)(__isl_take isl_map *map)
{
	return SF(isl_map_lexopt,SUFFIX)(map, 0);
}

__isl_give TYPE *SF(isl_map_lexmax,SUFFIX)(__isl_take isl_map *map)
{
	return SF(isl_map_lexopt,SUFFIX)(map, ISL_OPT_MAX);
}

__isl_give TYPE *SF(isl_set_lexmin,SUFFIX)(__isl_take isl_set *set)
{
	return SF(isl_map_lexmin,SUFFIX)(set);
}

__isl_give TYPE *SF(isl_set_lexmax,SUFFIX)(__isl_take isl_set *set)
{
	return SF(isl_map_lexmax,SUFFIX)(set);
}
