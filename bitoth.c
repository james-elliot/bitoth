#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define ABS(a) \
   ({ __typeof__ (a) _a = (a); \
     _a > 0 ? _a : -_a; })


#define CHECK_XY(x,y) (((x)>=0)&&((x)<8)&&((y)>=0)&&((y)<8))
#define IS_SET_XY(a,x,y) ((a)&((uint64_t)1<<((x)|((y)<<3))))
#define SET_XY(a,x,y) ((a)|=((uint64_t)1<<((x)|((y)<<3))))
#define CLEAR_XY(a,x,y) ((a)&= ~((uint64_t)1<<((x)|((y)<<3))))
#define FLIP_XY(a,x,y) ((a)^=((uint64_t)1<<((x)|((y)<<3))))

#define CHECK(x) (((x)>=0)&&((x)<64))
#define IS_SET(a,x) ((a)&((uint64_t)1<<((x))))
#define SET(a,x) ((a)|=((uint64_t)1<<((x))))
#define CLEAR(a,x) ((a)&= ~((uint64_t)1<<((x))))
#define FLIP(a,x) ((a)^=((uint64_t)1<<((x))))

#define NB_BITS(b)   ((int8_t)(__builtin_popcountl(b)))
#define TRAIL_BIT(b) (__builtin_ctzl(b))
#define LEAD_BIT(b)  (__builtin_clzl(b))
#define IND_BIT(b)   ((int8_t)(__builtin_ffsl((int64_t)b)))

bool get_out = false;
void handler(__attribute__((unused))int signum) {get_out=true;};

#define GET_OUT (-32767)
#define MAXV 32766
#define WIN 32700


uint64_t masks[64];
void init_masks() {
  for (int x=0;x<8;x++) {
    for (int y=0;y<8;y++) {
      int n = y*8+x;
      masks[n]=0;
      for (int dx=-1;dx<=1;dx++) {
        for (int dy=-1;dy<=1;dy++) {
          if CHECK_XY(x+dx,y+dy) {
              SET_XY(masks[n],x+dx,y+dy);
            }
        }
      }
    }
  }
}

int8_t dep[64][8][8];
void init_dep2() {
  for (int i=0;i<64;i++) {
    int x = i%8, y = i/8;
    int j=0;
    for (int dx=-1;dx<=1;dx++) {
      for (int dy=-1;dy<=1;dy++) {
        if ((dx!=0)||(dy!=0)) {
          int k=0;
          int nx=x,ny=y;
          while (true) {
            nx+=dx;ny+=dy;
            if (CHECK_XY(nx,ny)) dep[i][j][k]=(int8_t)(nx+(ny<<3));
            else break;
            k++;
          }
          if (k>=2) dep[i][j][k]=-1;
          else dep[i][j][0]=-1;
          j++;
        }
      }
    }
  }
}

typedef struct _dep_elm {
  uint8_t nb;
  uint8_t *t;
} dep_elm;
typedef struct _depl {
  uint8_t nb;
  dep_elm *tab;
} depl;
depl deps3[64];
void init_dep3() {
  for (int i=0;i<64;i++) {
    long unsigned int n=0;
    for (int j=0;j<8;j++) {
      if (dep[i][j][0]>=0) n++;
      }
    if (n>0) {
      deps3[i].nb=(uint8_t)n;
      deps3[i].tab=(dep_elm *)calloc(n,sizeof(dep_elm));
      int j=0;
      long unsigned int k=0;
      while (k<n) {
        if (dep[i][j][0]>=0) {
          long unsigned int nb;
          for (nb=0;nb<8;nb++) if (dep[i][j][nb]<0) break;
          deps3[i].tab[k].nb=(uint8_t)nb;
          deps3[i].tab[k].t=(uint8_t *)calloc(nb,sizeof(uint8_t));
          memcpy(deps3[i].tab[k].t,&dep[i][j][0],nb*sizeof(uint8_t));
          k++;
        }
        j++;
      }
    }
  }
}

bool play3(int x,uint64_t *myb,uint64_t *opb) {
  bool valid=false;
  for (int i=0;i<deps3[x].nb;i++) {
    if (!IS_SET(*opb,deps3[x].tab[i].t[0])) continue;
    int j;
    for (j=1;j<deps3[x].tab[i].nb;j++) {
      if (IS_SET(*opb,deps3[x].tab[i].t[j])) continue;
      if (IS_SET(*myb,deps3[x].tab[i].t[j])) {
        valid=true;
        int k=j-1;
        while (k>=0) {
          CLEAR(*opb,deps3[x].tab[i].t[k]);
          SET(*myb,deps3[x].tab[i].t[k]);
          k--;
        }
      }
      break;
    }
  }
  if (valid) SET(*myb,x);
  return valid;
}

int8_t **deps4[64];
uint64_t **masks4[64];
uint64_t *masks44[64];
uint64_t *masks45[64];
void init_deps4() {
  for (int i=0;i<64;i++) {
    long unsigned int n=0;
    for (int j=0;j<8;j++) {
      if (dep[i][j][0]>=0) n++;
      }
    if (n==0)
      deps4[i]=NULL;
    else {
      deps4[i]=(int8_t **)calloc(n+1,sizeof(int8_t*));
      masks4[i]=(uint64_t **)calloc(n,sizeof(uint64_t*));
      masks44[i]=(uint64_t *)calloc(n,sizeof(uint64_t));
      masks45[i]=(uint64_t *)calloc(n,sizeof(uint64_t));
      deps4[i][n]=NULL;
      int j=0;
      long unsigned int k=0;
      while (k<n) {
        if (dep[i][j][0]>=0) {
          long unsigned int nb;
          uint64_t masks[8];
          uint64_t mask=0;
          for (nb=0;nb<8;nb++) {
            if (dep[i][j][nb]<0) break;
            else {
              if (nb>0) {
                masks[nb]=masks[nb-1]|((uint64_t)1<<dep[i][j][nb]);
                mask|=((uint64_t)1<<dep[i][j][nb]);
              }
              else
                masks[nb]=((uint64_t)1<<dep[i][j][nb]);
	    }
	  }
	  deps4[i][k]=(int8_t *)calloc(nb+1,sizeof(int8_t));
          masks4[i][k]=(uint64_t *)calloc(nb,sizeof(uint64_t));
          memcpy(deps4[i][k],&dep[i][j][0],(nb+1)*sizeof(int8_t));
          memcpy(masks4[i][k],&masks[0],nb*sizeof(uint64_t));
          masks44[i][k]=mask;
          masks45[i][k]=masks[nb-1];
          k++;
        }
        j++;
      }
    }
  }
}

typedef struct
//__attribute__((packed))
_dep7 {
  int8_t t[8];
  uint64_t mask[8];
} dep7;
dep7 deps7[64][8];
void init_deps7() {
  bzero(deps7,sizeof(deps7));
  for (int i=0;i<64;i++) {
    int n=0;
    for (int j=0;j<8;j++) {
      if (dep[i][j][0]>=0) {
        for (int nb=0;nb<8;nb++) {
          deps7[i][n].t[nb]=dep[i][j][nb];
          if (dep[i][j][nb]<0) break;
          if (nb>0) deps7[i][n].mask[nb]=deps7[i][n].mask[nb-1]|((uint64_t)1<<dep[i][j][nb]);
          else deps7[i][n].mask[nb]=((uint64_t)1<<dep[i][j][nb]);
        }
        n++;
      }
    }
    deps7[i][n].t[0]=-1;
  }
}

bool play7(int x,uint64_t *myb,uint64_t *opb) {
  bool valid=false;
  dep7 *d=deps7[x];
  for (int i=0;i<8;i++) {
    int8_t *t=d[i].t;
    if (t[0]<0) break;
    if (IS_SET(*opb,t[0])) {
      for (int j=1;t[j]>=0;j++) {
        if (!IS_SET(*opb,t[j])) {
          if (IS_SET(*myb,t[j])) {
            valid=true;
            j--;
            *opb^=d[i].mask[j];
            *myb^=d[i].mask[j];
          }
          break;
        }
      }
    }
  }
  if (valid) SET(*myb,x);
  return valid;
}

bool play4(int x,uint64_t *myb,uint64_t *opb) {
  bool valid=false;
  int8_t** curi;
  curi=deps4[x];
  while (*curi!=NULL) {
    int8_t* curj=*curi;
    if (IS_SET(*opb,*curj)) {
      int8_t* orgj=curj;
      curj++;
      while (*curj>=0) {
        if (!IS_SET(*opb,*curj)) {
          if (IS_SET(*myb,*curj)) {
            valid=true;
            while(curj!=orgj) {
              curj--;
              CLEAR(*opb,*curj);
              SET(*myb,*curj);
            }
          }
          break;
        }
        curj++;
      }
    }
    curi++;
  }
  if (valid) SET(*myb,x);
  return valid;
}

bool play5(int x,uint64_t *myb,uint64_t *opb) {
  bool valid=false;
  int8_t** curi;
  curi=deps4[x];
  for (int i=0;curi[i]!=NULL;i++) {
    int8_t* curj=curi[i];
    if (IS_SET(*opb,*curj)) {
      for (int j=1;curj[j]>=0;j++) {
        if (!IS_SET(*opb,curj[j])) {
          if (IS_SET(*myb,curj[j])) {
            valid=true;
            j--;
            *opb^=masks4[x][i][j];
            *myb^=masks4[x][i][j];
          }
          break;
        }
      }
    }
  }
  if (valid) SET(*myb,x);
  return valid;
}

bool play8(int x,uint64_t *myb,uint64_t *opb) {
  bool valid=false;
  int8_t **curi=deps4[x],j;
  uint64_t *m44=masks44[x],*m45=masks45[x],m;
  int n;
  for (int i=0;curi[i]!=NULL;i++) {
    j=*curi[i];
    if (IS_SET(*opb,j)) {
      //    if (true) {
      m=*myb&m44[i];
      if (m>0) {
        if (j>x) {
          n=TRAIL_BIT(m);
          m=(((uint64_t)1<<n)-1);
        }
        else {
          n=64-LEAD_BIT(m);
          m=~(((uint64_t)1<<n)-1);
        }
	m&=m45[i];
        if ((*opb&m)==m) {
          valid=true;
          *opb^=m;
          *myb^=m;
        }
      }
    }
  }
  if (valid) SET(*myb,x);
  return valid;
}

bool play6(int x,uint64_t *myb,uint64_t *opb) {
  bool valid=false;
  int8_t** curi;
  int8_t** orgi;
  orgi=deps4[x];
  curi=orgi;
  while (*curi!=NULL) {
    int8_t* curj=*curi;
    if (IS_SET(*opb,*curj)) {
      int8_t* orgj=curj;
      curj++;
      while (*curj>=0) {
        if (!IS_SET(*opb,*curj)) {
          if (IS_SET(*myb,*curj)) {
            valid=true;
            curj--;
            uint64_t m=masks4[x][curi-orgi][curj-orgj];
            *opb^=m;
            *myb^=m;
          }
          break;
        }
        curj++;
      }
    }
    curi++;
  }
  if (valid) SET(*myb,x);
  return valid;
}

bool play2(int x,uint64_t *myb,uint64_t *opb) {
  bool valid=false;
  for (int i=0;i<8;i++) {
    if ((dep[x][i][0]>=0)&&IS_SET(*opb,dep[x][i][0])) {
      for (int j=1;dep[x][i][j]>=0;j++) {
        if IS_SET(*myb,dep[x][i][j]) {
            valid=true;
            for (int k=j-1;k>=0;k--) {
              CLEAR(*opb,dep[x][i][k]);
              SET(*myb,dep[x][i][k]);
            }
            break;
          }
        if (!IS_SET(*opb,dep[x][i][j])) break;
      }
    }
  }
  if (valid) SET(*myb,x);
  return valid;
}

bool play(int sq,uint64_t *myb,uint64_t *opb) {
  bool valid = false;
  int x=sq%8,y=sq/8;
//  if (IS_SET_XY(x,y,*myb | *opb)) return false;
  for (int dx=-1;dx<=1;dx++) {
    for (int dy=-1;dy<=1;dy++) {
      if ((dx==0)&&(dy==0)) continue;
      if (CHECK_XY(x+dx,y+dy) && IS_SET_XY(*opb,x+dx,y+dy)) {
        int nx=x+dx,ny=y+dy;
        do {
          nx+=dx;ny+=dy;
          if (!CHECK_XY(nx,ny)) break;
          if IS_SET_XY(*myb,nx,ny) {
              valid=true;
              nx-=dx;ny-=dy;
              while IS_SET_XY(*opb,nx,ny) {
                  CLEAR_XY(*opb,nx,ny);
                  SET_XY(*myb,nx,ny);
                  nx-=dx;ny-=dy;
                }
              break;
            }
          if (!IS_SET_XY(*opb,nx,ny)) break;
        } while (true);
      }
    }
  }
  if (valid) SET_XY(*myb,x,y);
  return valid;
}

int testable(uint64_t r,uint64_t myb,uint64_t opb,bool pl(int,uint64_t*,uint64_t*)) {
  uint64_t es = ~(myb|opb);
  int n=0;
  while (es) {
    int sq = IND_BIT(es)-1;
    FLIP(es,sq);
    uint64_t nmyb=myb,nopb=opb;
    if (pl(sq,&nmyb,&nopb)) n++;
    if (nmyb==nopb) n++;
    if (r==nmyb) n++;
  }
  return n;
}

bool playable(uint64_t myb,uint64_t opb) {
  uint64_t es = ~(myb|opb);
  while (es) {
    int sq = IND_BIT(es)-1;
    FLIP(es,sq);
    if (play8(sq,&myb,&opb)) return true;
  }
  return false;
}

int all_moves(uint64_t myb,uint64_t opb,int *moves) {
  uint64_t es = ~(myb|opb);
  int n=0;
  while (es) {
    uint64_t nmyb=myb,nopb=opb;
    int sq = IND_BIT(es)-1;
    FLIP(es,sq);
    if (play8(sq,&nmyb,&nopb)) moves[n++]=sq;
  }
  return n;
}

int eval_lib(uint64_t b,uint64_t es) {
  int libs=0;
  while(b) {
    int sq = IND_BIT(b)-1;
    b^=(uint64_t)1<<sq;
    libs+=NB_BITS(es&masks[sq]);
  }
  return libs;
}

#define VAL_C (50)
#define VAL_OC (-20)
#define VAL_LC (-5)

int eval_corner(uint64_t myb,uint64_t opb,int c,int oc,int cl, int cr) {
  int ev = 0;
  if (IS_SET(myb,c)) return VAL_C;
  if (!IS_SET(myb|opb,c)) {
    if (IS_SET(myb,oc)) return VAL_OC;
    if (IS_SET(myb,cl)) ev+=VAL_LC;
    if (IS_SET(myb,cr)) ev+=VAL_LC;
  }
  return ev;
}

int eval_pos(uint64_t myb,uint64_t opb) {
  int ev = 0;
  ev+=eval_corner(myb,opb, 0, 9, 1, 8);
  ev+=eval_corner(myb,opb, 7,14, 6,15);
  ev+=eval_corner(myb,opb,56,49,48,57);
  ev+=eval_corner(myb,opb,63,54,62,55);
  ev-=eval_corner(opb,myb, 0, 9, 1, 8);
  ev-=eval_corner(opb,myb, 7,14, 6,15);
  ev-=eval_corner(opb,myb,56,49,48,57);
  ev-=eval_corner(opb,myb,63,54,62,55);
  return ev;
}

int16_t eval(uint64_t myb,uint64_t opb) {
  uint64_t es = ~(myb|opb);
//  int nb = NB_BITS(es);
//  int vdisc = (NB_BITS(myb)-NB_BITS(opb));
//  if (nb<=12) return vdisc;
  int vpos = eval_pos(myb,opb);
  int vlib = -(eval_lib(myb,es)-eval_lib(opb,es));
  return (int16_t)(vpos+vlib);
}


void display(uint64_t wb,uint64_t bb) {
  for (int j=0;j<=7;j++) {
    fprintf(stderr,"%2d ",8*j);
    for (int i=0;i<8;i++) {
      if IS_SET_XY(wb,i,j) fprintf(stderr," X ");
      else if IS_SET_XY(bb,i,j) fprintf(stderr," O ");
      else fprintf(stderr," . ");
    }
    fprintf(stderr,"\n");
  }
  fprintf(stderr,"    0  1  2  3  4  5  6  7\n");
  fprintf(stderr,"eval_pos=%d\n",eval_pos(wb,bb));
}

#define NB_BITS_H 27
#define SIZE_H ((uint64_t)1<<NB_BITS_H)
#define MASK_H (SIZE_H-1)

typedef
struct
__attribute__((packed))
_hash_t {
  int8_t bmove;
  uint64_t myb;
  uint64_t opb;
  int8_t pass;
  int16_t v_sup,v_inf;
  uint8_t base;
  uint8_t dist;
} hash_t;
hash_t *hashv;

#define PASS (-2)
#define INVALID_MOVE (-1)

bool retrieve_v_hash(uint64_t myb,uint64_t opb,int8_t pass,uint32_t ind,
                    int16_t *v_inf,int16_t *v_sup,int8_t *bmove,uint8_t dist) {
  if ((hashv[ind].opb==opb)&&(hashv[ind].myb==myb)&&
      (hashv[ind].pass==pass)) {
    if ((hashv[ind].dist==dist)
        ||
        ((hashv[ind].v_inf==hashv[ind].v_sup)&&(ABS(hashv[ind].v_inf)>=WIN))
) {
      *v_inf=hashv[ind].v_inf;
      *v_sup=hashv[ind].v_sup;
      *bmove=hashv[ind].bmove;
      return true;
    }
    *bmove=hashv[ind].bmove;
    return false;
  }
  return false;
}

void store_v_hash_both(uint64_t myb,uint64_t opb,int8_t pass,
                       uint32_t ind,int16_t v,
                       uint8_t dist,uint8_t base,int8_t move) {
  if ((hashv[ind].base!=base)||(hashv[ind].dist<=dist)) {
    hashv[ind].v_inf=v;
    hashv[ind].v_sup=v;
    hashv[ind].myb=myb;
    hashv[ind].opb=opb;
    hashv[ind].pass=pass;
    hashv[ind].base=base;
    hashv[ind].bmove=move;
    hashv[ind].dist=dist;
    };
}

void store_v_hash(uint64_t myb,uint64_t opb,
                  int8_t pass,uint32_t ind,
                  int16_t alpha,int16_t beta,int16_t g,
                  uint8_t dist,uint8_t base,int8_t move) {
  if ((hashv[ind].base!=base)||(hashv[ind].dist<=dist)) {
    if ((hashv[ind].myb!=myb) ||(hashv[ind].opb!=opb)
        ||(hashv[ind].pass!=pass) || (hashv[ind].dist!=dist)) {
      /* Not an update. Have to initialize/reset everything */
      hashv[ind].v_inf=-MAXV;
      hashv[ind].v_sup=MAXV;
      hashv[ind].dist=dist;
      hashv[ind].myb=myb;
      hashv[ind].opb=opb;
      hashv[ind].pass=pass;
    }
    hashv[ind].base=base;
    hashv[ind].bmove=move;
    if ((g>alpha)&&(g<beta)) {hashv[ind].v_inf=g;hashv[ind].v_sup=g;}
    else if (g<=alpha) hashv[ind].v_sup=g;
    else if (g>=beta) hashv[ind].v_inf=g;
    };
}

uint32_t t_myb[4][65536],t_opb[4][65536],t_pass;
void init_idx() {
  for (int i=0;i<4;i++)
    for (int j=0;j<65536;j++) {
      t_myb[i][j]=(uint32_t)((uint64_t)lrand48()&MASK_H);
      t_opb[i][j]=(uint32_t)((uint64_t)lrand48()&MASK_H);
    }
  t_pass=(uint32_t)((uint64_t)lrand48()&MASK_H);
}

void init_hash() {
  hashv = (hash_t *)calloc(SIZE_H,sizeof(hash_t));
  if (hashv==NULL) {
    fprintf(stderr,"Error allocating memory\n");
    exit(-1);
  }
}

void init_all() {
  init_dep2();
  init_dep3();
  init_deps4();
  init_deps7();
  init_idx();
  init_hash();
  init_masks();
}

uint32_t compute_ind(uint64_t myb,uint64_t opb,bool pass) {
  uint32_t ind = 0;
  for (int i=0;i<4;i++) {
    ind ^= t_myb[i][myb&0xffff];
    myb = myb >> 16;
    ind ^= t_opb[i][opb&0xffff];
    opb = opb >> 16;
  }
  if (pass) ind ^= t_pass;
  return ind;
}

int best_move;

int16_t ab(uint64_t myb,uint64_t opb,
       int16_t alpha,int16_t beta,
       uint8_t depth,uint8_t base,uint8_t maxdepth,
       bool pass) {
  uint64_t es = ~(myb|opb);
  if (es==0) {
    int16_t v = NB_BITS(myb)-NB_BITS(opb);
    if (v>0) v=WIN+v;
    else if (v<0) v=-WIN+v;
    return v;
  }
  if (depth==maxdepth) {
    return eval(myb,opb);
  }
  int16_t v=-MAXV;
  bool lpass=true;
  int8_t lmove=INVALID_MOVE,nsq=INVALID_MOVE;
  uint32_t ind = compute_ind(myb,opb,pass);
  int16_t v_inf,v_sup;
  if (retrieve_v_hash(myb,opb,pass,ind,&v_inf,&v_sup,&nsq,maxdepth-depth)) {
    if (depth==base) {best_move=nsq;}
    if (v_inf==v_sup) return v_inf; /* Exact evaluation */
    if (v_inf>=beta) return v_inf; /* Beta cut */
    if (v_sup<=alpha)  return v_sup; /* Alpha cut */
    alpha=MAX(alpha,v_inf);
    beta=MIN(beta,v_sup);
  }
  int8_t sq;
  int16_t a=alpha;
  if (nsq==PASS) goto pass;
  while (es) {
    if (nsq>=0) {
      sq=nsq;
      nsq=INVALID_MOVE;
    }
    else
      sq=IND_BIT(es)-1;
    FLIP(es,sq);
    uint64_t nmyb=myb,nopb=opb;
    if (play8(sq,&nmyb,&nopb)) {
      lpass=false;
      int16_t nv = -ab(nopb,nmyb,-beta,-a,depth+1,base,maxdepth,false);
      if (get_out) return GET_OUT;
      if (nv>v) {
        v=nv;
        lmove=sq;
        if (depth==base) best_move=sq;
        a=MAX(a,v);
        if (a>=beta) goto fin;
      }
    }
  }
pass:
  if (lpass) {
    lmove=PASS;
    if (pass) {
      v = NB_BITS(myb)-NB_BITS(opb);
      if (v>0) v=WIN+v;
      else if (v<0) v=-WIN+v;
      store_v_hash_both(myb,opb,pass,ind,v,maxdepth-depth,base,PASS);
      return v;
    }
    else {
      v = -ab(opb,myb,-beta,-a,depth+1,base,maxdepth,true);
      if (get_out) return GET_OUT;
    }
  }
fin:
  store_v_hash(myb,opb,pass,ind,alpha,beta,v,maxdepth-depth,base,lmove);
  return v;
}

void set_pos(char *name,uint64_t *wb,uint64_t *bb) {
  char *s=NULL;
  long unsigned int n=0;
  FILE *fp=fopen(name,"r");
  for (int j=7;j>=0;j--) {
    long int ret=getline(&s,&n,fp);
    if (ret==-1) {
      fprintf(stderr,"getline failed\n");
      exit(-1);
    }
    fprintf(stderr,"%s",s);
    for (int i=0;i<=7;i++) {
      switch (s[4+3*i]) {
      case 'X' :
        SET_XY(*wb,i,j);
        break;
      case 'O' :
        SET_XY(*bb,i,j);
        break;
      case '.' :
        break;
      default:
        fprintf(stderr,"Zorglub!!!!\n");
        exit(-1);
        }
    }
  }
  fclose(fp);
  fprintf(stderr,"End read\n");
  free(s);
}

int main(int argc, char **argv) {
  struct itimerval timer={{0,0},{0,0}};
  uint64_t myb=0,opb=0;
  init_all();
  if ((argc<3)||(argc>4)) {
    fprintf(stderr,"Bad number of arguments\n");
    exit(-1);
  }
  
  int player=atoi(argv[1]);
  if ((player!=1)&&(player!=2)) {
    fprintf(stderr,"Bad player argument\n");
    exit(-1);
  }

  int time_play=atoi(argv[2]);
  if ((time_play<=0)||(time_play>=120)) {
    fprintf(stderr,"Bad time argument\n");
    exit(-1);
  }

  
  if (argc==4) {
    set_pos(argv[3],&myb,&opb);
    /*
    uint64_t n;
    n=0;
    clock_t t1;
    double f1=0,f2=0,f3=0,f4=0,f5=0,f6=0,f7=0,f,f8=0;
    int nb=10000000;
    for (uint64_t j=0;j<10;j++) {
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,myb,opb,play);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f1+=f;
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,myb,opb,play2);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f2+=f;
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,myb,opb,play3);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f3+=f;
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,myb,opb,play4);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f4+=f;
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,myb,opb,play5);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f5+=f;
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,myb,opb,play6);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f6+=f;
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,myb,opb,play7);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f7+=f;
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,myb,opb,play8);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f8+=f;
      fprintf(stderr,"%15ld f1=%6.2f f2=%6.2f f3=%6.2f f4=%6.2f f5=%6.2f f6=%6.2f f7=%6.2f f8=%6.2f\n",
             n,f1,f2,f3,f4,f5,f6,f7,f8);
    }
    exit(-1);
    */
  }
  else {
    if (player==1) {
      SET_XY(opb,3,4);
      SET_XY(opb,4,3);
      SET_XY(myb,3,3);
      SET_XY(myb,4,4);
    }
    else {
      SET_XY(myb,3,4);
      SET_XY(myb,4,3);
      SET_XY(opb,3,3);
      SET_XY(opb,4,4);
    }
  }
  display(myb,opb);
  uint8_t depth=0;
  bool opp_pass=false;
  int16_t evals[128];
  signal(SIGALRM,handler);
  while (playable(myb,opb)||playable(opb,myb)) {
    if ((player==1)&&(playable(myb,opb))) {
      clock_t time=clock();
      int nb_free=NB_BITS(~(myb|opb));
      fprintf(stderr,"nb_free=%d\n",nb_free);
      int16_t alpha=-MAXV,beta=MAXV,res;
      timer.it_value.tv_sec=time_play;
      timer.it_value.tv_usec=0;
      setitimer(ITIMER_REAL,&timer,NULL);
      get_out=false;
      int old_best=INVALID_MOVE;
      for (uint8_t maxdepth=depth+2;;maxdepth++) {
      back:
	best_move=INVALID_MOVE;
	res = ab(myb,opb,alpha,beta,depth,depth,maxdepth,opp_pass);
	double ftime=(double)(clock()-time)/(double)CLOCKS_PER_SEC;
	fprintf(stderr,"alpha=%6d beta=%6d depth=%3d maxdepth=%3d move=%3d res=%6d time=%f\n",
		alpha,beta,depth,maxdepth,best_move,res,ftime);
	if (res==GET_OUT) {
	  if (best_move==INVALID_MOVE) best_move=old_best;
	  break;
	}
	else {
	  old_best=best_move;
	  evals[maxdepth]=res;
	  if (best_move<0){
	    fprintf(stderr,"Invalid move=%d\n",best_move);
	    exit(-1);
	  }
	  if ((res<=alpha)||(res>=beta)) {alpha=res-1;beta=res+1;goto back;}
	  if (abs(res)>WIN) break;
	  if (((res>=evals[maxdepth-1])&&(maxdepth%2==1)) ||
	      ((res<=evals[maxdepth-1])&&(maxdepth%2==0))) {
	    alpha=res-3;beta=res+3;
	  }
	  else {
	    alpha=evals[maxdepth-1]-3;beta=evals[maxdepth-1]+3;
	  }
	}
      }
      fprintf(stderr,"my_move=%d\n",best_move);
      printf("%d\n",best_move);
      fflush(stdout);
      play8(best_move,&myb,&opb);
      display(myb,opb);
    }
    player=1;
    depth++;
    if (playable(opb,myb)) {
      opp_pass=false;
      int move;
      int moves[64];
      int nb=all_moves(opb,myb,moves);
      fprintf(stderr,"moves:");
      for (int i=0;i<nb;i++)
	fprintf(stderr,"%3d",moves[i]);
      fprintf(stderr,"\n");
      char *s=NULL;
      long unsigned int n=0;
      do {
	fprintf(stderr,"your_move:");
	fflush(stderr);
	long int ret=getline(&s,&n,stdin);
	if (ret==-1) {
	  fprintf(stderr,"getline failed\n");
	  exit(-1);
	}
	move=atoi(s);
	fprintf(stderr,"move=%d\n",move);
      } while ((!CHECK(move))||(IS_SET(myb|opb,move))||(!play8(move,&opb,&myb)));
      display(myb,opb);
      free(s);
    }
    else opp_pass=true;
    depth++;
  }
  fprintf(stderr,"res=%d\n",NB_BITS(myb)-NB_BITS(opb));
  return 0;
}
