#include <stdlib.h>
#include "string.h"
#include <stdio.h>
//#define _POSIX_C_SOURCE >= 199309L
#include <sys/time.h>


typedef struct arbr arbr;


struct arbr
{
 //a pointer to a string which is the label of the father edge (root do not need this)
 unsigned char* edglabel;   //oui mais besoin d'un deuxième label (celui du noeud)?
 //Pointers toward subtrees. There is at most 256 possible subtrees, depending on the first character of the outcoming edge, which is a byte (this way, multibytes characters are considered as many characters, but we probably don’t care).
 arbr* subtree[256]; //moyen de def juste un pointeur et def la taille plus tard?
 //Is this node a terminal node (equivalent to existence of a son with an edge only labelled $). //pb si on veut label ":" et "$"
 //int isend;
};

char *getAlphabet(char* text, int textsize)
{
 int i,j;
 unsigned char alphabet[textsize];
 int alphabetSize=0;
 for (i=0;i<textsize;i++)
 {
  int isAdd=0;
  for (j=0;j<alphabetSize;j++)
  {
   if (text[i]==alphabet[j]) 
    isAdd=1;
  }
  if(!(isAdd))
  {
   alphabet[alphabetSize]=text[i];
   alphabetSize+=1;
  }
 }
 unsigned char *alphabet2=malloc(alphabetSize*sizeof(unsigned char));//malloc?
 for (i=0;i<alphabetSize;i++)
 {
  alphabet2[i]=alphabet[i];
 }
 return alphabet2;
}

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
 
 struct timespec t0, t1, t2, t3;
 double d0, d1, d2;

 printf("Trying naive approach\n");
// clock_gettime(CLOCK_REALTIME, &t0);
 naif(patsize, textsize, argv[1], argv[2]);
 printf("Trying RKM approach\n");
// clock_gettime(CLOCK_REALTIME, &t1);
 rkm(256, 1027, patsize, textsize, argv[1], argv[2]);
 printf("Trying KMP approach\n");
// clock_gettime(CLOCK_REALTIME, &t2);
 kmp(patsize, textsize, argv[1], argv[2]);
// clock_gettime(CLOCK_REALTIME, &t3);

 //this is supposed to be the time taken by each of these approaches. Until now, always outputs 0
 d0 = ((double)t1.tv_sec+1.0e-9*t1.tv_nsec)-((double)t0.tv_sec+1.0e-9*t0.tv_nsec);
 d1 = ((double)t2.tv_sec+1.0e-9*t2.tv_nsec)-((double)t1.tv_sec+1.0e-9*t1.tv_nsec);
 d2 = ((double)t3.tv_sec+1.0e-9*t3.tv_nsec)-((double)t2.tv_sec+1.0e-9*t2.tv_nsec);

 printf("%f, %f, %f\n", d0, d1, d2);  

 //We should add a $ at the end of the text here…
 char* text2= malloc((textsize+1)*(sizeof(char)));
 strcpy(text2, argv[2]);
 text2[textsize]='$';
 struct arbr* tree=malloc(sizeof(arbr));
 createSuffTree(textsize+1, text2, tree);
// printf("Launching research of motif %s in the suffix tree of %s$\n",argv[1],argv[2]);
 char *alphabet=getAlphabet(text2,textsize+1);
 int i;
// printf("\n\nBegin print alphabet\n");
// for(i=0;alphabet[i];i++){
//     printf("I am the num %d and have the value %d (corresp to char %c)\n",i+1,alphabet[i],alphabet[i]);
// }
 matchSuffTree(patsize, argv[1], tree, textsize+1, alphabet);
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
   printf("found pattern at shift %d\n", i);
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
int rkm(int alpsize, int mod, int patsize, int textsize, unsigned char* pat, unsigned char* text)
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



//up to which point are two strings equals? Useful to know where to insert a new node when constructing the tree.
int strmtch(int l1, int l2, unsigned char* s1, unsigned char* s2)
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
void initarbr(struct arbr* a, unsigned char* s)
{
 //a->isend = 0;
// char str[strlen(s)];
// strcpy(str, s);
 a->edglabel=malloc((strlen(s)+1)*sizeof(unsigned char)); 
 strcpy(a->edglabel, s);
 int k;
 //a->subtree=(arbr*) malloc(256*sizeof(arbr*));
 for (k=0; k<256; k++)
 {
  a->subtree[k]=NULL;
 }
}


int createSuffTree(int textsize, unsigned char* text, arbr* root)
{
 //struct arbr root=*rt;
 initarbr(root, "");
 struct arbr* currTree=root;
 int i,k;
 //loop through all the suffixes
 for (i=0; i<textsize; i++)
 {
//  printf("\nTaking care of %d° suffix : %s (first letter correspond to %d)\n",i+1, &text[i], text[i]);
  int j=0;
  while (j<textsize-i)
  {
//   printf("\nremains %d° suffix : %s (first letter correspond to %d)\n",j , &text[i+j], text[i+j]);
   if (currTree==NULL)
   {
    printf("Oops, current tree is NULL. This shall not happen!\n");
    printf("i=%d, j=%d, suffix to match = %s\n", i, j, &text[i+j]);
    return 0;
    //j=textsize-1;
    //currTree=root;
   }
   else if (currTree->subtree[text[i+j]]==NULL)
   {
//    printf("NULL create new node\n");
    // create new son with label text[i+j:len] and add to the father's childrens at indice "text[i+j]" (first letter of the suffix) 
    struct arbr* newSon= malloc(sizeof(arbr)); // if not malloc, the memory adress don't change at each turn of the loop
    initarbr(newSon, &text[i+j]);
    currTree->subtree[text[i+j]]= newSon;
//    printf("put the new node at the letter %c and the ind %d\n",text[i+j],text[i+j]);
    // change j to end while loop
    j=textsize-i;
//    printf("print first node (must stay constant) %s\n", root->subtree[text[0]]->edglabel);
    //aim achieved, don't forget to move back to the root
    currTree=root;
   }
   else
   {//we have a matching edge (at least in part) -> we have two possibilities :
    // 1- the edge's label is included in the suffix, so we need to move forward in the tree
    // 2- the suffix is included in the edge's label, so we need to cut the edge
    int lenSuffix=textsize-i-j;
//    printf("There is a subtree %p, … but does it have an edglabel?\n", &currTree->subtree[text[i+j]]);
    int lenEdgLab=strlen(currTree->subtree[text[i+j]]->edglabel);
//    printf("Apparently yes!\n");
//    printf("cut or move forward?\n");
    int lenMatch=strmtch(lenSuffix, lenEdgLab, &text[i+j], currTree->subtree[text[i+j]]->edglabel);
//    printf("lenSuffix(rl)=%d, lenEdgLab(el)=%d, lenMatch(lq)=%d\n", lenSuffix, lenEdgLab, lenMatch);
//    printf("edge label : %s\n",currTree->subtree[text[i+j]]->edglabel);
//    printf("edge label must begin by : %c\n", text[i+j]);
    if (lenMatch==lenEdgLab&&lenMatch==lenSuffix)
    {//add the $ and do further suffixes
//     printf("exact match!\n");
     j+=lenEdgLab;
     currTree=root;
    }
    else if (lenMatch==lenEdgLab)
    {//option 1 : move forward
//     printf("forth\n");
     if (currTree->subtree[text[i+j]]==NULL)
     {
      printf("Something went wrong, we are mooving to nowhere!\n");
      printf("i=%d, j=%d, suffix to match=%s\n", i, j, &text[i+j]);
      return 0;
     }
     currTree=currTree->subtree[text[i+j]];
     j+=lenEdgLab;
    }
    else
    {//option 2 : cut the concerned edge
//     printf("cut\n");
     unsigned char* strbg=malloc((lenMatch+1)*sizeof(unsigned char));
     strncpy(strbg, currTree->subtree[text[i+j]]->edglabel, lenMatch);
     unsigned char* strend=malloc((lenEdgLab-lenMatch+1)*sizeof(unsigned char));
     strcpy(strend, &currTree->subtree[text[i+j]]->edglabel[lenMatch]);
     //cut current edge and create a new son of current Node
//     printf("Cutting %s into %s and %s\n", currTree->subtree[text[i+j]]->edglabel, strbg, strend);
     struct arbr* newSon=malloc(sizeof(arbr));
     initarbr(newSon, strbg);
     //create a new son of the precedent son
     struct arbr* newGrandson = malloc(sizeof(arbr));
     initarbr(newGrandson, &text[i+j+lenMatch]);
     
     //change edge label of the old son
//     printf("change edge label at %d (corr to %c) by %s\n",text[i+j],text[i+j],strbg);
     currTree->subtree[text[i+j]]->edglabel=strend;
     
     //move old son to the newSon to avoid losing its sons
     newSon->subtree[strend[0]]=currTree->subtree[text[i+j]];
     //taking care of link father and son
     newSon->subtree[text[i+j+lenMatch]]=newGrandson;
     currTree->subtree[text[i+j]]=newSon;
     //j+=lenMatch;
     
     //aim achieved, don't forget to move back to the root
     currTree=root;
     j=textsize-i;
     //currTree=currTree->subtree[text[i+j]];
     
//     printf("print first node (may have change if cut occurs in it) %s\n", root->subtree[text[0]]->edglabel);
    }
    
    //char* ss = currTree->subtree[text[i+j]]->edglabel;
   // j++;
   }
  }
 }
 return 1;
}


matchSuffTree(int patsize, unsigned char *pattern, arbr *tree, int textsize, char *alphabet)
{
 int j=0;
 arbr *currTree=tree;
 while (j<patsize)
  {
//   printf("\nround %d : state of the search : %s (first letter correspond to %d)\n",j , &pattern[j], pattern[j]);
   if (currTree==NULL)
   {
    printf("Oops, current tree is NULL. This shall not happen!\n");
    printf("j=%d, patternto match = %s\n", j, &pattern[j]);
    return 1;
   }
   else if (currTree->subtree[pattern[j]]==NULL)
   {
    printf("Pattern not found (no matching son)\n");
    return 0;
   }
   else
   {//we have a matching edge (at least in part) -> we have two possibilities :
    // 1- the edge's label is included in the suffix, so we need to move forward in the tree
    // 2- the suffix is included in the edge's label, so we need to cut the edge
    int lenRemainPat=patsize-j;
//    printf("There is a subtree %p, … but does it have an edglabel?\n", &currTree->subtree[pattern[j]]);
    int lenEdgLab=strlen(currTree->subtree[pattern[j]]->edglabel);
//    printf("Apparently yes!\n");
//    printf("stay (final node for the pattern) or move forward?\n");
    int lenMatch=strmtch(lenRemainPat, lenEdgLab, &pattern[j], currTree->subtree[pattern[j]]->edglabel);
//    printf("lenSuffix(rl)=%d, lenEdgLab(el)=%d, lenMatch(lq)=%d\n", lenRemainPat, lenEdgLab, lenMatch);
//    printf("edge label : %s\n",currTree->subtree[pattern[j]]->edglabel);
//    printf("edge label must begin by : %c\n", pattern[j]);
    if (lenMatch==lenEdgLab&&lenMatch==lenRemainPat)
    {
//     printf("pattern found\n");
     findPositions(currTree->subtree[pattern[j]], 0, textsize, patsize, alphabet);
     return 0;
    }
    else if (lenMatch==lenEdgLab)
    {//option 1 : move forward
//     printf("forth\n");
     if (currTree->subtree[pattern[j]]==NULL)
     {
      printf("Something went wrong, we are mooving to nowhere!\n");
      printf("j=%d, pattern to match=%s\n", j, &pattern[j]);
      return 1;
     }
     currTree=currTree->subtree[pattern[j]];
     j+=lenEdgLab;
    }
    else if(lenMatch==lenRemainPat)
    {
//     printf("pattern found, moving to %s \n",currTree->subtree[pattern[j]]->edglabel);
     currTree=currTree->subtree[pattern[j]];
     findPositions(currTree, lenEdgLab-lenMatch, textsize, patsize, alphabet);
     return 0;
    }
    else
    {
     printf("Pattern not found\n");
     return 0;
    }
   }
  }
}

int *travelTree(arbr *tree, int *posArray, unsigned char *alphabet, int nbCalls, int posArraySize)
{
 //int posArraySize= count(posArray);//(sizeof(posArray) / sizeof(posArray[0])) -1;
 //if reach a leaf extend posArray
 if (nbCalls>1)
  posArray[posArraySize-1]+=strlen(tree->edglabel);
// printf("progress %s %d\n",tree->edglabel,posArraySize);
 int i=0;
 int bool=0;
 int cptBrEmpty=0;
 for(i=0;alphabet[i];i++)//Do not work with the null character! …null character is the string termination in C. It cannot work with it aniway…
 {
//  printf("Take care of %d (%c)\n",i,alphabet[i]);
  if (tree->subtree[alphabet[i]]!=NULL)
  {
   bool=1;
   posArray=travelTree(tree->subtree[alphabet[i]], posArray, alphabet,nbCalls+1, posArraySize);
   posArraySize= posArray[0];//sizeof(posArray) / sizeof(posArray[0]) -1;
//   printf("%d\n", posArraySize);
  }
  else
  {
//   printf("…it is null\n");
   cptBrEmpty+=1;
  }
 }
 if (bool==0)
 {
//  printf("I reach a leaf %s\n",tree->edglabel);
  posArray=realloc( posArray, (posArraySize+1)*sizeof(int) );
  posArray[posArraySize]=posArray[posArraySize-1];
  posArraySize+=1;
  posArray[0]=posArraySize;
 }
 posArray[posArraySize-1] -= strlen(tree->edglabel);
 if (nbCalls==1)
{
 int *posArrayFin=malloc((posArraySize-1)*sizeof(int));
  for(i=0;i<posArraySize-1;i++)
      posArrayFin[i]=posArray[i];
  free(posArray);
  return posArrayFin;
 }
 return posArray;
}

int findPositions(arbr *tree, int lenNotMatching,int textsize,int patsize, char *alphabet)
{
 //By counting the leafs at the end of the current tree we have the number of positions
 //To have matching positions we need to reach all the leafs which represents the end of the text
 //We need to sum the lengths of all the nodes to a leaf to have the end position of one match
 int *posArray= malloc( 2*sizeof(int) );
// printf("lenNotMatching : %d\n",lenNotMatching);
 posArray[0]=2;
 posArray[1]=lenNotMatching;
 posArray=travelTree(tree, posArray,alphabet,1, 2);
 int i;
 for(i=1;i<posArray[0]-1;i++){
  posArray[i]=textsize - posArray[i];
  printf("pattern found at index %d to %d\n", posArray[i]-patsize,posArray[i]);
 }
 printf("%d occurence(s) found\n",posArray[0]-2);
}

