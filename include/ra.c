#include "../include/ra.h"

static void indent_print(const char *format,...);
static void upInd();
static void downInd();

void printExp(Condition *expr) {
   switch(expr->t) {
      case RA_COND_EQ:
         printf("%s = %s", expr->expr.comp.col1, expr->expr.comp.col2);
         break;
      case RA_COND_LT:
         printf("%s < %s", expr->expr.comp.col1, expr->expr.comp.col2);
         break;
      case RA_COND_GT:
         printf("%s > %s", expr->expr.comp.col1, expr->expr.comp.col2);
         break;
      case RA_COND_LEQ:
         printf("%s <= %s", expr->expr.comp.col1, expr->expr.comp.col2);
         break;
      case RA_COND_GEQ:
         printf("%s >= %s", expr->expr.comp.col1, expr->expr.comp.col2);
         break;
      case RA_COND_AND:
         printExp(expr->expr.binary.expr1);
         printf(" and ");
         printExp(expr->expr.binary.expr2);
         break;
      case RA_COND_OR:
         printExp(expr->expr.binary.expr1);
         printf(" or ");
         printExp(expr->expr.binary.expr2);
         break;
      case RA_COND_NOT:
         printf("not (");
         printExp(expr->expr.unary.expr);
         printf(")");
         break;
      default:
         puts("Unknown expression type");
   }
}

void printRA(RA *ra) {
   int i;
   switch(ra->t) {
      case RA_TABLE:
         indent_print("Table(%s)", ra->ra.table.name);
         break;
      case RA_SELECT:
         indent_print("Select(");
         printExp(ra->ra.select.expr);
         printf(", ");
         upInd();
         printRA(ra->ra.select.ra);
         downInd();
         indent_print(")");
         break;
      case RA_PROJECT:
         indent_print("Project([");
         for (i=0; i<ra->ra.project.num_cols; ++i) {
            if (i != 0) printf(", ");
            printf("%s", ra->ra.project.cols[i]);
         }
         printf("], ");
         upInd();
         printRA(ra->ra.project.ra);
         downInd();
         indent_print(")");
         break;
      case RA_UNION:
         indent_print("Union(");
         upInd();
         printRA(ra->ra.binary.ra1);
         indent_print(", ");
         printRA(ra->ra.binary.ra2);
         downInd();
         indent_print(")");
         break;
      case RA_DIFFERENCE:
         indent_print("Difference(");
         upInd();
         printRA(ra->ra.binary.ra1);
         indent_print(", ");
         printRA(ra->ra.binary.ra2);
         downInd();
         indent_print(")");
         break;
      case RA_CROSS:
         indent_print("Cross(");
         upInd();
         printRA(ra->ra.binary.ra1);
         indent_print(", \n");
         printRA(ra->ra.binary.ra2);
         downInd();
         indent_print(")");
         break;
      case RA_RENAME:
         indent_print("Rename(");
         printf("%s, ", ra->ra._rename.table_name);
         printf("[");
         for (i=0; i<ra->ra._rename.num_col_names; ++i) {
            if (i != 0) indent_print(", ");
            printf("%s", ra->ra._rename.col_names[i]);
         }
         printf("], ");
         upInd();
         printRA(ra->ra._rename.ra);
         downInd();
         indent_print(")");
         break;
      default:
         puts("Unknown RA type");
   }
}

RA *Table (const char *name) {
   RA *new_ra = (RA *)calloc(1, sizeof(RA));
   new_ra->t = RA_TABLE;
   new_ra->ra.table.name = strdup(name);
   return new_ra;
}

RA *Select (RA *ra, Condition *expr) {
   RA *new_ra = (RA *)calloc(1, sizeof(RA));
   new_ra->t = RA_SELECT;
   new_ra->ra.select.expr = expr;
   new_ra->ra.select.ra = ra;
   return new_ra;
}

RA *Project (RA *ra, unsigned num_cols, ...) {
   size_t i;
   va_list argp;
   RA *new_ra = (RA *)calloc(1, sizeof(RA));
   new_ra->t = RA_PROJECT;
   new_ra->ra.project.ra = ra;
   new_ra->ra.project.num_cols = num_cols;
   new_ra->ra.project.cols = (char **)calloc(num_cols, sizeof(char *));
   va_start(argp, num_cols);
   for (i=0; i<num_cols; ++i) {
      new_ra->ra.project.cols[i] = strdup(va_arg(argp, const char *));
   }
   return new_ra;
}

RA *Union (RA *ra1, RA *ra2) {
   RA *new_ra = (RA *)calloc(1, sizeof(RA));
   new_ra->t = RA_UNION;
   new_ra->ra.binary.ra1 = ra1;
   new_ra->ra.binary.ra2 = ra2;
   return new_ra;
}

RA *Difference (RA *ra1, RA *ra2) {
   RA *new_ra = (RA *)calloc(1, sizeof(RA));
   new_ra->t = RA_DIFFERENCE;
   new_ra->ra.binary.ra1 = ra1;
   new_ra->ra.binary.ra2 = ra2;
   return new_ra;
}

RA *Cross (RA *ra1, RA *ra2) {
   RA *new_ra = (RA *)calloc(1, sizeof(RA));
   new_ra->t = RA_CROSS;
   new_ra->ra.binary.ra1 = ra1;
   new_ra->ra.binary.ra2 = ra2;
   return new_ra;
}

RA *Rename (RA *ra, const char *table_name, unsigned num_col_names, ...) {
   size_t i, j=0;
   va_list argp;
   va_start(argp, num_col_names);
   RA *new_ra = (RA *)calloc(1, sizeof(RA));
   new_ra->t = RA_RENAME;
   new_ra->ra._rename.table_name = strdup(table_name);
   new_ra->ra._rename.num_col_names = num_col_names;
   if (num_col_names > 0) {
      new_ra->ra._rename.col_names = (char **)calloc(num_col_names, sizeof(char *));
      for (i=0; i<num_col_names; ++i)
         new_ra->ra._rename.col_names[j++] = strdup(va_arg(argp, const char *));
   }
   new_ra->ra._rename.ra = ra;
   return new_ra;
}

Condition *Eq(const char *col1, const char *col2) {
   Condition *new_expr = (Condition *)calloc(1, sizeof(Condition));
   new_expr->t = RA_COND_EQ;
   new_expr->expr.comp.col1 = strdup(col1);
   new_expr->expr.comp.col2 = strdup(col2);
   return new_expr;
}

Condition *Lt(const char *col1, const char *col2) {
   Condition *new_expr = (Condition *)calloc(1, sizeof(Condition));
   new_expr->t = RA_COND_LT;
   new_expr->expr.comp.col1 = strdup(col1);
   new_expr->expr.comp.col2 = strdup(col2);
   return new_expr;
}

Condition *Gt(const char *col1, const char *col2) {
   Condition *new_expr = (Condition *)calloc(1, sizeof(Condition));
   new_expr->t = RA_COND_GT;
   new_expr->expr.comp.col1 = strdup(col1);
   new_expr->expr.comp.col2 = strdup(col2);
   return new_expr;
}

Condition *Leq(const char *col1, const char *col2) {
   Condition *new_expr = (Condition *)calloc(1, sizeof(Condition));
   new_expr->t = RA_COND_LEQ;
   new_expr->expr.comp.col1 = strdup(col1);
   new_expr->expr.comp.col2 = strdup(col2);
   return new_expr;
}

Condition *Geq(const char *col1, const char *col2) {
   Condition *new_expr = (Condition *)calloc(1, sizeof(Condition));
   new_expr->t = RA_COND_GEQ;
   new_expr->expr.comp.col1 = strdup(col1);
   new_expr->expr.comp.col2 = strdup(col2);
   return new_expr;
}

Condition *And(Condition *expr1, Condition *expr2) {
   Condition *new_expr = (Condition *)calloc(1, sizeof(Condition));
   new_expr->t = RA_COND_AND;
   new_expr->expr.binary.expr1 = expr1;
   new_expr->expr.binary.expr2 = expr2;
   return new_expr;
}

Condition *Or(Condition *expr1, Condition *expr2) {
   Condition *new_expr = (Condition *)calloc(1, sizeof(Condition));
   new_expr->t = RA_COND_OR;
   new_expr->expr.binary.expr1 = expr1;
   new_expr->expr.binary.expr2 = expr2;
   return new_expr;
}

Condition *Not(Condition *expr) {
   Condition *new_expr = (Condition *)calloc(1, sizeof(Condition));
   new_expr->t = RA_COND_NOT;
   new_expr->expr.unary.expr = expr;
   return new_expr;
}

void deleteRA(RA *ra) {
   int i;
   switch(ra->t) {
      case RA_SELECT:
         deleteCondition(ra->ra.select.expr);
         deleteRA(ra->ra.select.ra);
         break;
      case RA_PROJECT:
         deleteRA(ra->ra.project.ra);
         for (i=0; i<ra->ra.project.num_cols; ++i)
            free(ra->ra.project.cols[i]);
         break;
      case RA_UNION:
      case RA_DIFFERENCE:
      case RA_CROSS:
         deleteRA(ra->ra.binary.ra1);
         deleteRA(ra->ra.binary.ra2);
         break;
      case RA_RENAME:
         deleteRA(ra->ra._rename.ra);
         for (i=0; i<ra->ra._rename.num_col_names; ++i)
            free(ra->ra._rename.col_names[i]);
         break;
      case RA_TABLE:
         free(ra->ra.table.name);
         break;
   }
   free(ra);
}

void deleteCondition(Condition *expr) {
   switch (expr->t) {
      case RA_COND_EQ:
      case RA_COND_LEQ:
      case RA_COND_GEQ:
      case RA_COND_GT:
      case RA_COND_LT:
         free(expr->expr.comp.col1);
         free(expr->expr.comp.col2);
         break;
      case RA_COND_AND:
      case RA_COND_OR:
         deleteCondition(expr->expr.binary.expr1);
         deleteCondition(expr->expr.binary.expr2);
         break;
      case RA_COND_NOT:
         deleteCondition(expr->expr.unary.expr);
         break;
   }
   free(expr);
}

#define BUF_SIZE 5000
static int ind = 0;

static void upInd() {
   ind++;
   printf("\n");
}

static void downInd() {
   ind--;
   printf("\n");
   if (ind < 0) printf("error, ind is < 0");
}

static void indent_print(const char *format,...)
{
   /* indent */
   int i;
   va_list argptr;
   char buffer[BUF_SIZE];
   if (ind < 1) ind = 0;
   for (i=0; i<ind; ++i) {
      if (i == 0)
         sprintf(buffer, "\t");
      else
         strcat(buffer, "\t");
   }
   va_start(argptr, format);
   vsnprintf(buffer + ind, BUF_SIZE, format, argptr);
   va_end(argptr);
   fputs(buffer, stdout);
   fflush(stdout);
}

int ra_main(int argc, char const *argv[])
{
   RA *ra1 = Project(Select(Table("bazzle"), And(Eq("foo", "bar"), Lt("popo", "toto"))), 
               3, "foo", "bar", "baz"),
      *ra2 = Project(Rename(Select(Cross(Rename(Table("Foo"), "f", 1, "Col1"),
                        Rename(Table("Foo"), "g", 1, "Col2")),Not(Eq("Col1","Col2"))
                  ),"res", 2, "Col1", "Col2"),2, "Col1", "Col2");
   printRA(ra1);
   printRA(ra2);
   deleteRA(ra1);
   deleteRA(ra2);
   return 0;
}