#include <stdlib.h>
#include "string.h"
#include <stdio.h>
#include "time.h"


//first argument given to the program is the pattern, second argument is the text.
int main (int argc, char** argv)
{
 if (argc!=3)
 {
  printf("Expected 2 arguments, got %d.\n", argc-1);
  return 0;
 }
 int patsize = strlen(argv[1]);
 int textsize = strlen(argv[2]);
 time_t t0, t1, t2, t3;
 double d0, d1, d2;

 printf("Trying naive approach\n");
 t0 = time(&t0);
 naif(patsize, textsize, argv[1], argv[2]);
 printf("Trying RKM approach\n");
 t1 = time(&t1);
 rkm(4, 29, patsize, textsize, argv[1], argv[2]);
 printf("Trying KMP approach\n");
 t2 = time(&t2);
 kmp(patsize, textsize, argv[1], argv[2]);
 t3 = time(&t3);

 //this is supposed to be the time taken by each of these approaches. Until now, always outputs 0
 d0 = difftime(t1, t0);
 d1 = difftime(t2, t1);
 d2 = difftime(t3, t2);

 printf("%f, %f, %f\n", d0, d1, d2);  

 suff(textsize, argv[2]);

 return 1;
}


//computing alpᵉˣᵖ modulo mod for RKM algorithm
int powmod(int alp, int exp, int mod)
{
 int intr;
 if (exp==1)
 {
  return (alp%mod);
 }
 if (exp==0)
 {
  return 1;
 }
 if ((exp%2)==0)
 {
  intr = powmod(alp, exp/2, mod);
  return ((intr*intr)%mod);
 }
 else
 {
 intr = powmod(alp, exp-1, mod);
 return ((intr*alp)%mod);
 }
}


//The naive approach: first two arguments are sizes, last two are texts (pattern then text)
int naif(int patsize, int textsize, char* pat, char* text)
{
 int i, j;
 int ret = 0;
 for (i=0; i<textsize-patsize+1; i++)
 {
  for (j=0; j<patsize; j++)
  {
   if (pat[j]!=text[i+j])
   {
    break;
   }
  }
  if (j==patsize)
  {
   printf("found pattern at index %d\n", i);
   ret=1;
  }
 }
 return ret;
}

//naive subroutine with shift, for RKM algorithm
int nnaif(int patsize, int shift, char* pat, char* text)
{
 int j;
 int ret = 0;
 for (j=0; j<patsize; j++)
 {
  if (pat[j]!=text[j])
  {
   break;
  }
 }
 if (j==patsize)
 {
  printf("found pattern at index %d\n", shift);
  ret=1;
 }
 return ret;
}


//RKM matcher. First argument is the size of the alphabet, second argument is the modulo used.
//The value of each character is always the corresponding byte (e.g. 'A'≡108, 'C'≡110, 'G'≡114, 'T'≡127, 'U'≡128), whatever the size of the alphabet given in argument.
int rkm(int alpsize, int mod, int patsize, int textsize, char* pat, char* text)
{
 int h = powmod(alpsize, patsize-1, mod); //réélement besoin de cette fonction?
 int i;
 int p=0;
 int tt=0;
 //pattern preprocess
 for (i=0; i<patsize; i++)
 {
  p=((alpsize*p+pat[i])%mod);
  tt=((alpsize*tt+text[i])%mod);
 }
 //printf("%d: preprocessed from pattern\n", p);
 //printf("%d: text begin\n", tt);
 int s;
 for (s=0; s<textsize-patsize; s++)
 {
  if(p==tt)
  {
   printf("pattern may occure at shift %d\n", s);
   nnaif(patsize, s, pat, &text[s]);
  }
  tt=((alpsize*(tt-h*text[s])+text[s+patsize])%mod);
  if (tt<0) //remind that C “%” operator may return negative resul..
  {
   tt+=mod;
  }
//  printf("%d at shift %d\n", tt, s+1);
 }
               //besoin d'un tour de plus?
 if(p==tt)
 {
  printf("pattern may occure at shift %d\n", s);
  nnaif(patsize, s, pat, &text[s]);
 }
 return 1;
}


//compute the prefix function of the pattern for KMP matcher, and fill it into table prefix
prefix_function(int patsize, char* pat, int* prefix)
{
 int q;
 int k=0;
 prefix[0]=0;
 for(q=1; q<patsize; q++)
 {
  while (k>0&&pat[k]!=pat[q])
  {
   k=prefix[k-1];
  }
  if(pat[k]==pat[q])
  {
   k++;
  }
  prefix[q]=k;
 }
 return 1;
}

//KMP matcher
kmp(int patsize, int textsize, char* pat, char* text)
{
 int prefix[patsize];
 prefix_function(patsize, pat, prefix);
 int q=0;
 int i;
 for (i=0; i<textsize; i++)
 {
  while (q>0&&pat[q]!=text[i])
  {
   q=prefix[q-1];
  }
  if (pat[q]==text[i])
  {
   q++;
  }
  if (q==patsize)
  {
   printf("pattern found at shift %d\n", i-patsize+1);
   q=prefix[q-1];
  }
 }
}

//(Trying to) define a structure of tree in order to build a prefix tree.

typedef struct arbr arbr;

struct arbr
{
 //a pointer to a string which is the label of the father edge (root do not need this)
 char* edglabel;   //oui mais besoin d'un deuxième label (celui du noeud)?
 //Pointers toward subtrees. There is at most 256 possible subtrees, depending on the first character of the outcoming edge, which is a byte (this way, multibytes characters are considered as many characters, but we probably don’t care).
 arbr* subtree[256]; //moyen de def juste un pointeur et def la taille plus tard?
 //Is this node a terminal node (equivalent to existence of a son with an edge only labelled $). //pb si on veut label ":" et "$"
 //int isend;
};


//up to which point are two strings equals? Useful to know where to insert a new node when constructing the tree.
int strmtch(int l1, int l2, char* s1, char* s2)
{
 if (l1==0||l2==0)
 {
  return 0;
 }//pas besoin du if
 int i=0;
 while (i<l1&&i<l2)
 {
  if (s1[i]==s2[i])
  {
   i++;
  }
  else
  {
   return i;
  }
 }
 return i;
}

//initializing a (already declared) tree of one node, father edge being labelled by second argument
//TODO: copy the string to set the edge label
void initarbr(struct arbr* a, char* s)
{
 //a->isend = 0;
// char str[strlen(s)];
// strcpy(str, s);
 a->edglabel=malloc((strlen(s)+1)*sizeof(char)); 
 strcpy(a->edglabel, s);
 int k;
 for (k=0; k<256; k++)
 {
  a->subtree[k]=NULL;
 }
}

//(Trying to) build a suffix tree of the text. TODO… well, almost everything
suff(int textsize, char* text)
{
 //printf("insuff\n");
 struct arbr a;
 int k;
 initarbr(&a, "$");
 //printf("coucou\n");
 //printf ("%s\n", a.edglabel);
 struct arbr* curr=&a;
 int i;
 //printf("enteringmainloop\n");
 for (i=0; i<textsize; i++)
 {
  //printf("i=%d\n",i);
  int j=0;
  //printf("enteringwhileloop\n");
  while (j<textsize-i)
  {
   printf("%s\n", &text[i+j]);
   //printf("j=%d\n",j);
   if (curr->subtree[text[i+j]]==NULL)
   {
    printf("NULL\n");
    struct arbr b;
    initarbr(&b, &text[i+j]);
    //b.isend=1;
    curr->subtree[text[i+j]]= &b;
    j=textsize-i;
   }
   else
   {
    int rl=textsize-i-j;
    int el=strlen(curr->subtree[text[i+j]]->edglabel);
    printf("coucou\n");
    int lq=strmtch(rl, el, &text[i+j], curr->subtree[text[i+j]]->edglabel);
    printf("rl=%d, el=%d, lq=%d\n", rl, el, lq);
    printf("%s\n",curr->subtree[text[i+j]]->edglabel);
    printf("%c\n", text[i+j]);
    if (lq==el)
    {
     printf("forth\n");
     j+=el;
     curr=curr->subtree[text[i+j]];
    }
    else if (lq==rl)
    {
     printf("Error, misplaced “$” in text\n");
    }
    else
    {
     printf("cut\n");
     //use library copy functions
     char strbg[lq];
     for (k=0; k<lq; k++)
     {
      strbg[k]=curr->subtree[text[i+j]]->edglabel[k];
     }
     char* strend=malloc((el-lq+1)*sizeof(char));
     for (k=0; k<el-lq; k++)
     {
      strend[k]=curr->subtree[text[i+j]]->edglabel[k+lq];
     }

     struct arbr b;
     initarbr(&b, strbg);
     struct arbr c;
     initarbr(&c, &text[i+j+lq]);
     curr->subtree[text[i+j]]->edglabel=strend;
     
     b.subtree[strend[0]]=curr->subtree[text[i+j]];
     b.subtree[text[i+j+lq]]=&c;
     curr->subtree[text[i+j]]=&b;
     j+=lq;
     curr=curr->subtree[text[i+j]];
    }
    


    //printf("test%d\n", curr->subtree[text[i+j]]);
    //printf("test%d\n", NULL);
   // printf("unimplementedcase\n");
    //char* ss = curr->subtree[text[i+j]]->edglabel;
   // printf("%s\n", *curr->subtree[text[i+j]]->edglabel);
   // j++;
   }
  }
 }
}


