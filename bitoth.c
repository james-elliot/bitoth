#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define WIN 32700
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

#define CHECK(x,y) (((x)>=0)&&((x)<8)&&((y)>=0)&&((y)<8))

#define IS_SET_XY(a,x,y) ((a)&((uint64_t)1<<((x)|((y)<<3))))
#define SET_XY(a,x,y) ((a)|=((uint64_t)1<<((x)|((y)<<3))))
#define CLEAR_XY(a,x,y) ((a)&= ~((uint64_t)1<<((x)|((y)<<3))))
#define FLIP_XY(a,x,y) ((a)^=((uint64_t)1<<((x)|((y)<<3))))

#define IS_SET(a,x) ((a)&((uint64_t)1<<((x))))
#define SET(a,x) ((a)|=((uint64_t)1<<((x))))
#define CLEAR(a,x) ((a)&= ~((uint64_t)1<<((x))))
#define FLIP(a,x) ((a)^=((uint64_t)1<<((x))))

#define NB_BITS(b)   (__builtin_popcountl(b))
#define TRAIL_BIT(b) (__builtin_ctzl(b))
#define LEAD_BIT(b)  (__builtin_clzl(b))

uint64_t masks[64];

void init_masks() {
  for (int x=0;x<8;x++) {
    for (int y=0;y<8;y++) {
      int n = y*8+x;
      masks[n]=0;
      for (int dx=-1;dx<=1;dx++) {
        for (int dy=-1;dy<=1;dy++) {
          if CHECK(x+dx,y+dy) {
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
            if (CHECK(nx,ny)) dep[i][j][k]=nx+(ny<<3);
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
    int n=0;
    for (int j=0;j<8;j++) {
      if (dep[i][j][0]>=0) n++;
      }
    if (n>0) {
      deps3[i].nb=n;
      deps3[i].tab=(dep_elm *)calloc(n,sizeof(dep_elm));
      int j=0,k=0;
      while (k<n) {
        if (dep[i][j][0]>=0) {
          int nb;
          for (nb=0;nb<8;nb++) if (dep[i][j][nb]<0) break;
          deps3[i].tab[k].nb=nb;
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
    int n=0;
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
      int j=0,k=0;
      while (k<n) {
        if (dep[i][j][0]>=0) {
          int nb;
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
  int8_t** curi;
  curi=deps4[x];
  for (int i=0;curi[i]!=NULL;i++) {
    int8_t j=*curi[i];
    if (IS_SET(*opb,j)) {
      uint64_t m=*myb&masks44[x][i];
      if (m>0) {
        int n;
        if (j>x) {
          n=TRAIL_BIT(m);
          m=(((uint64_t)1<<n)-1);
        }
        else {
          n=64-LEAD_BIT(m);
          m=~(((uint64_t)1<<n)-1);
        }
        m&=masks45[x][i];
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

bool play(int x,int y,uint64_t *myb,uint64_t *opb) {
  bool valid = false;
//  if (IS_SET_XY(x,y,*myb | *opb)) return false;
  for (int dx=-1;dx<=1;dx++) {
    for (int dy=-1;dy<=1;dy++) {
      if ((dx==0)&&(dy==0)) continue;
      if (CHECK(x+dx,y+dy) && IS_SET_XY(*opb,x+dx,y+dy)) {
        int nx=x+dx,ny=y+dy;
        do {
          nx+=dx;ny+=dy;
          if (!CHECK(nx,ny)) break;
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

bool playb(int sq,uint64_t *myb,uint64_t *opb) {
  return play(sq%8,sq/8,myb,opb);
}

int testable(uint64_t r,uint64_t myb,uint64_t opb,bool pl(int,uint64_t*,uint64_t*)) {
  uint64_t es = ~(myb|opb);
  int n=0;
  while (es) {
    int sq = __builtin_ffsl(es)-1;
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
    int sq = __builtin_ffsl(es)-1;
    FLIP(es,sq);
    if (play2(sq,&myb,&opb)) return true;
  }
  return false;
}

int eval_lib(uint64_t b,uint64_t es) {
  int libs=0;
  while(b) {
    int sq = __builtin_ffsl(b)-1;
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
  ev+=eval_corner(myb,opb,0,9,1,8);
  ev+=eval_corner(myb,opb,7,14,6,15);
  ev+=eval_corner(myb,opb,56,49,48,57);
  ev+=eval_corner(myb,opb,63,54,62,55);
  ev-=eval_corner(opb,myb,0,9,1,8);
  ev-=eval_corner(opb,myb,7,14,6,15);
  ev-=eval_corner(opb,myb,56,49,48,57);
  ev-=eval_corner(opb,myb,63,54,62,55);
  return ev;
}

int eval(uint64_t myb,uint64_t opb) {
  uint64_t es = ~(myb|opb);
//  int nb = NB_BITS(es);
//  int vdisc = (NB_BITS(myb)-NB_BITS(opb));
//  if (nb<=12) return vdisc;
  int vpos = eval_pos(myb,opb);
  int vlib = -(eval_lib(myb,es)-eval_lib(opb,es));
  return vpos+vlib;
}


void display(uint64_t wb,uint64_t bb) {
  for (int j=7;j>=0;j--) {
    printf("%2d ",10*j);
    for (int i=0;i<8;i++) {
      if IS_SET_XY(wb,i,j) printf(" X ");
      else if IS_SET_XY(bb,i,j) printf(" O ");
      else printf(" . ");
    }
    printf("\n");
  }
  printf("    0  1  2  3  4  5  6  7\n");
  printf("eval_pos=%d\n",eval_pos(wb,bb));
}

#define NB_BITS_H 25
#define SIZE_H ((uint64_t)1<<NB_BITS_H)
#define MASK_H (SIZE_H-1)

struct
__attribute__((packed))
_hash_t {
  int8_t move;
  uint64_t myb;
  uint64_t opb;
  int16_t v_sup;
  int16_t v_inf;
  uint8_t d;
  uint8_t base_d;
  uint8_t delta_d;
};
typedef struct _hash_t hash_t;
hash_t *hashv;

#define NO_EVAL 32767
#define PASS (-2)
int8_t find_hash(uint64_t myb,uint64_t opb,uint32_t ind,int16_t *v_inf,
              int16_t *v_sup,int delta_depth) {
  if ((hashv[ind].myb==myb)&&(hashv[ind].opb==opb)) {
    if ((delta_depth==hashv[ind].delta_d) ||
        ((hashv[ind].v_inf==hashv[ind].v_sup)&&(ABS(hashv[ind].v_sup)>WIN))) {
      *v_inf=hashv[ind].v_inf;
      *v_sup=hashv[ind].v_sup;
      }
    return hashv[ind].move;
  }
  return -1;
}

void store_hash(uint64_t myb,uint64_t opb,uint32_t ind,int8_t move,
                int16_t v_inf,int16_t v_sup,int basedepth,int depth,
                int delta_depth) {
  if ((hashv[ind].base_d<basedepth)||(hashv[ind].d>=depth)) {
    if ((v_inf!=-NO_EVAL)||(hashv[ind].delta_d!=delta_depth)) hashv[ind].v_inf=v_inf;
    if ((v_sup!= NO_EVAL)||(hashv[ind].delta_d!=delta_depth)) hashv[ind].v_sup=v_sup;
    hashv[ind].myb=myb;
    hashv[ind].opb=opb;
    hashv[ind].move=move;
    hashv[ind].d=depth;
    hashv[ind].base_d=basedepth;
    hashv[ind].delta_d=delta_depth;
  }
}

uint32_t t_myb[4][65536],t_opb[4][65536];
void init_idx() {
  for (int i=0;i<4;i++)
    for (int j=0;j<65536;j++) {
      t_myb[i][j]=lrand48()&MASK_H;
      t_opb[i][j]=lrand48()&MASK_H;
    }
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

uint32_t compute_ind(uint64_t myb,uint64_t opb) {
  uint32_t ind = 0;
  for (int i=0;i<4;i++) {
    ind ^= t_myb[i][myb&0xffff];
    myb = myb >> 16;
    ind ^= t_opb[i][opb&0xffff];
    opb = opb >> 16;
  }
  return ind;
}

int best_move=-1;

int ab(uint64_t myb,uint64_t opb,
       int16_t alpha,int16_t beta,
       int depth,int basedepth,int maxdepth,
       bool pass) {
  int16_t v=-NO_EVAL,v_inf=-NO_EVAL,v_sup=NO_EVAL;
  bool lpass=true;
  int8_t lmove=-1;
  uint32_t ind = compute_ind(myb,opb);
  int8_t nsq=find_hash(myb,opb,ind,&v_inf,&v_sup,maxdepth-depth);
  if (v_inf==v_sup) {
    if (depth==basedepth) best_move=nsq;
    return v_inf;
  }
  if (v_inf>=beta) {
    if (depth==basedepth) best_move=nsq;
    return v_inf;
  }
  if (v_sup<=alpha) {
    if (depth==basedepth) best_move=nsq;
    return v_sup;
  }
  if (nsq==PASS) goto pass;
  uint64_t es = ~(myb|opb);
  if (es==0) {
    v = NB_BITS(myb)-NB_BITS(opb);
    if (v>0) v=WIN+v;
    else if (v<0) v=-WIN+v;
    store_hash(myb,opb,ind,PASS,v,v,basedepth,depth,maxdepth-depth);
    return v;
  }
  if (depth>=maxdepth) return eval(myb,opb);
  int8_t sq;
  int16_t a=MAX(v_inf,alpha);
  while (es) {
    if (nsq>=0) {
      sq=nsq;
      nsq=-1;
    }
    else
      sq=__builtin_ffsl(es)-1;
    FLIP(es,sq);
    uint64_t nmyb=myb,nopb=opb;
    if (play8(sq,&nmyb,&nopb)) {
      lpass=false;
      if (lmove==-1) lmove=sq;
      int16_t nv = -ab(nopb,nmyb,-beta,-a,depth+1,basedepth,maxdepth,false);
      if (nv>v) {
        v=nv;
        lmove=sq;
        if (depth==basedepth) best_move=sq;
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
      store_hash(myb,opb,ind,PASS,v,v,basedepth,depth,maxdepth-depth);
      return v;
    }
    else {
      v = -ab(opb,myb,-beta,-a,depth+1,basedepth,maxdepth,true);
    }
  }
fin:
  if (v>=beta) {
    store_hash(myb,opb,ind,lmove,-NO_EVAL,v,basedepth,depth,maxdepth-depth);
  }
  else if (v<=alpha) {
    store_hash(myb,opb,ind,lmove,v,NO_EVAL,basedepth,depth,maxdepth-depth);
  }
  else {
    store_hash(myb,opb,ind,lmove,v,v,basedepth,depth,maxdepth-depth);
  }
  return v;
}

void set_pos(char *name,uint64_t *wb,uint64_t *bb) {
  char *s=NULL;
  long unsigned int n;
  FILE *fp=fopen(name,"r");
  for (int j=7;j>=0;j--) {
    getline(&s,&n,fp);
    printf("%s",s);
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
        printf("Zorglub!!!!\n");
        exit(-1);
        }
    }
  }
  fclose(fp);
  printf("End read\n");
}

int main(int argc, char **argv) {
  uint64_t wb=0,bb=0;
  init_all();
  if (argc==2) {
    set_pos(argv[1],&wb,&bb);
    /*
    uint64_t n;
    n=0;
    clock_t t1;
    double f1=0,f2=0,f3=0,f4=0,f5=0,f6=0,f7=0,f,f8=0;
    int nb=10000000;
    for (uint64_t j=0;j<10;j++) {
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,wb,bb,playb);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f1+=f;
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,wb,bb,play2);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f2+=f;
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,wb,bb,play3);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f3+=f;
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,wb,bb,play4);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f4+=f;
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,wb,bb,play5);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f5+=f;
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,wb,bb,play6);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f6+=f;
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,wb,bb,play7);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f7+=f;
      t1=clock();
      for (uint64_t i=j*nb;i<(j+1)*nb;i++) n+=testable(i,wb,bb,play8);
      f=(double)(clock()-t1)/(double)CLOCKS_PER_SEC;
      f8+=f;
      printf("%15ld f1=%6.2f f2=%6.2f f3=%6.2f f4=%6.2f f5=%6.2f f6=%6.2f f7=%6.2f f8=%6.2f\n",
             n,f1,f2,f3,f4,f5,f6,f7,f8);
    }
    exit(-1);
    */
  }
  else {
    SET_XY(bb,3,4);
    SET_XY(bb,4,3);
    SET_XY(wb,3,3);
    SET_XY(wb,4,4);
  }
  display(wb,bb);
  int x,y;
  int depth=0;
  while (playable(wb,bb)||playable(bb,wb)) {
      if (playable(wb,bb)) {
        int alpha=-32766,beta=32766,res;
        clock_t time=clock();
        int nb_free=NB_BITS(~(wb|bb));
        printf("nb_free=%d\n",nb_free);
        if (nb_free<=-1) {
          res = ab_fast(wb,bb,-2,2,depth,depth,false);
//          res = ab(wb,bb,-2,2,depth,depth,10000,false);
          x = best_move%8;y = best_move/8;
          double ftime=(double)(clock()-time)/(double)CLOCKS_PER_SEC;
          printf("move=%3d res=%6d time=%f\n",x+y*10,res,ftime);
        }
        else {
          for (int maxdepth=depth+2;;maxdepth++) {
          back:
            res = ab(wb,bb,alpha,beta,depth,depth,maxdepth,false);
            x = best_move%8;y = best_move/8;
            double ftime=(double)(clock()-time)/(double)CLOCKS_PER_SEC;
            printf("alpha=%6d beta=%6d depth=%3d maxdepth=%3d move=%3d res=%6d time=%f\n",
                   alpha,beta,depth,maxdepth,x+y*10,res,ftime);
            if (ftime>1.0) break;
            if (res<=alpha) {alpha=-32766;beta=res+1;goto back;}
            if (res>=beta) {alpha=res-1;beta=32766;goto back;}
            if (abs(res)>WIN) break;
            if ((maxdepth-depth)%2==0) {alpha=res-10;beta=res+1;}
            else {alpha=res-1;beta=res+10;}
          }
        }
        uint64_t nwb=wb,nbb=bb;
        printf("coucou=%d\n",x+10*y);
        play8(x+8*y,&nwb,&nbb);
        play(x,y,&wb,&bb);
        display(wb,bb);
        fflush(stdout);
        if ((nwb!=wb)||(nbb!=bb)) {
          printf("zorglab\n");
          display(nwb,nbb);
          exit(-1);
        }
      }
      depth++;
      if (playable(bb,wb)) {
        do {
          char *s=NULL;
          long unsigned int n;
          int move;
          getline(&s,&n,stdin);
          move=atoi(s);
          x = move%10;y=move/10;
          printf("x=%d y=%d\n",x,y);
        } while ((!CHECK(x,y))||(IS_SET_XY(wb|bb,x,y))||(!play(x,y,&bb,&wb)));
        display(wb,bb);
      }
      depth++;
    }
  printf("res=%d\n",NB_BITS(wb)-NB_BITS(bb));
  return 0;
}
/*
x=4,y=6
p=52
*/
