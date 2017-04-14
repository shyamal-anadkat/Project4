#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

//******Project4 : Locking user libs (part2)*******//
typedef long Align;

union header {
  struct {
    union header *ptr;
    uint size;
  } s;
  Align x;
};

typedef union header Header;

//where malloc calls free and you have multiple locks
static struct spinlock mlock; //malloc lock declare
static struct spinlock flock; //free lock declare
static Header base;
static Header *freep;

void
free(void *ap)
{
  //only init if not already init.
  if (!flock.init) {spin_init(&flock);}

  Header *bp, *p;

  bp = (Header*)ap - 1;

  // lock free lock - finegrained ?
  spin_lock(&flock);

  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if(bp + bp->s.size == p->s.ptr){
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp){
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;

  //unlock free lock
  spin_unlock(&flock);
}

static Header*
morecore(uint nu)
{
  char *p;
  Header *hp;

  if(nu < 4096)
    nu = 4096;
  p = sbrk(nu * sizeof(Header));
  if(p == (char*)-1)
    return 0;
  hp = (Header*)p;
  hp->s.size = nu;
  free((void*)(hp + 1));
  return freep;
}

void*
malloc(uint nbytes)
{

  //init if mlock not init previously held
  if (!mlock.init) {spin_init(&mlock);}

  //init if flock not init previously held
  if (flock.init != 1) {spin_init(&flock);}

  Header *p, *prevp;
  uint nunits;

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;

  //lock flock and mlock after init
  spin_lock(&mlock);
  spin_lock(&flock);

  //START ATOMIC
  if((prevp = freep) == 0){
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }

  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
    if(p->s.size >= nunits){
      if(p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      freep = prevp;

      //END ATOMIC
      
      //unlock before returning 
      spin_unlock(&flock);
      spin_unlock(&mlock);

      return (void*)(p + 1);
    }
    if(p == freep) {

      //unlock flock before calling morecore which calls free
      spin_unlock(&flock);

      if((p = morecore(nunits)) == 0) {
      //unlock mlock before return 
      spin_unlock(&mlock);
        return 0;
      }
    }
  }
}