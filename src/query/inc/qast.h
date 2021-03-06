/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TDENGINE_TAST_H
#define TDENGINE_TAST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <tskiplist.h>
#include "os.h"

#include "taosmsg.h"
#include "taosdef.h"
#include "tvariant.h"

struct tSQLSyntaxNode;
struct SSchema;
struct tSkipList;
struct tSkipListNode;

enum {
  TSQL_NODE_EXPR = 0x1,
  TSQL_NODE_COL = 0x2,
  TSQL_NODE_VALUE = 0x4,
};

typedef bool (*__result_filter_fn_t)(const void *, void *);
typedef void (*__do_filter_suppl_fn_t)(void *, void *);

/**
 * this structure is used to filter data in tags, so the offset of filtered tag column in tagdata string is required
 *
 */
typedef struct tQueryInfo {
  int32_t       offset;   // offset value in tags
  int32_t       colIdx;   // index of column in schema
  uint8_t       optr;     // expression operator
  SSchema       sch;      // schema of tags
  tVariant      q;        // query condition value on the specific schema, filter expression
  __compar_fn_t compare;  // filter function
} tQueryInfo;

typedef struct SBinaryFilterSupp {
  __result_filter_fn_t   fp;
  __do_filter_suppl_fn_t setupInfoFn;
  void *                 pExtInfo;
} SBinaryFilterSupp;

typedef struct tSQLSyntaxNode {
  uint8_t nodeType;
  union {
    struct {
      uint8_t optr;  // filter operator
      uint8_t hasPK; // 0: do not contain primary filter, 1: contain
      void *  info;  // support filter operation on this expression only available for leaf node
      
      struct tSQLSyntaxNode *pLeft;  // left child pointer
      struct tSQLSyntaxNode *pRight; // right child pointer
    } _node;
    struct SSchema *pSchema;
    tVariant *      pVal;
  };
} tSQLSyntaxNode;


typedef struct tQueryResultset {
  void ** pRes;
  int64_t num;
} tQueryResultset;

void tSQLBinaryExprFromString(tSQLSyntaxNode **pExpr, SSchema *pSchema, int32_t numOfCols, char *src, int32_t len);

void tSQLBinaryExprToString(tSQLSyntaxNode *pExpr, char *dst, int32_t *len);

void tSQLBinaryExprDestroy(tSQLSyntaxNode **pExprs, void (*fp)(void*));

void tSQLBinaryExprTraverse(tSQLSyntaxNode *pExpr, SSkipList *pSkipList, SArray *result, SBinaryFilterSupp *param);

void tSQLBinaryExprCalcTraverse(tSQLSyntaxNode *pExprs, int32_t numOfRows, char *pOutput, void *param, int32_t order,
                                char *(*cb)(void *, char *, int32_t));

void tSQLBinaryExprTrv(tSQLSyntaxNode *pExprs, int32_t *val, int16_t *ids);
void tQueryResultClean(tQueryResultset *pRes);

uint8_t getBinaryExprOptr(SSQLToken *pToken);

#ifdef __cplusplus
}
#endif

#endif  // TDENGINE_TAST_H
