/* 
	Copyright © 2013 Reagan Lopez
	[This program is licensed under the "MIT License"]
	Please see the file LICENSE in the source
	distribution of this software for license terms	
*/

/*****************************************************************/
// main.cpp: Program to derive and simplify a function f(x).
// Author: Reagan Lopez
// Date: 07/29/2013
/*****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

const char* OUTPUTFILE = "diff.txt"; // output filename
ofstream outfile;
char NextChar;
char LatestChar;

// each node has 1 of these class states:
// a Literal, an Identifier (for variable), or an Operator.
// Parenthesized expressions have been reduced.
typedef enum { Literal, Identifier, Operator } NodeClass;
typedef struct NodeType * NodePtr;	// forward announcement
// using the forward declared pointer type: NodePtr
typedef struct NodeType
{
	NodeClass	Class;			// 1 of the 3 classes.
	char		Symbol;		// store: Identifier, Operator
	int			LitVal;		// if Literal, this is its value
	NodePtr		Left;			// subtree
	NodePtr		Right;			// subtree
} s_node_tp;

// forward declarations of functions.
NodePtr Make( NodeClass Class, char Symbol, int value, NodePtr Left, NodePtr Right );
NodePtr Copy( NodePtr Root );
void PrintTree( NodePtr Root );
NodePtr Expression();
NodePtr Term();
NodePtr Factor();
NodePtr Primary();
NodePtr Derive( NodePtr Root );
NodePtr Simplify( NodePtr Root );
bool IsEqual( NodePtr Left, NodePtr Right );
bool IsLit( char sym, NodePtr node );
bool BothLit( NodePtr node1, NodePtr node2 );

/*****************************************************************/
// Function to fetch next character.
/*****************************************************************/
char GetNextChar()
{
    NextChar = getchar();
	if ( ' ' == NextChar ) {
        GetNextChar();
	}
	else {
        outfile << NextChar;
    }

    return NextChar;
}

/*****************************************************************/
// Function to make a node.
// malloc() new node from heap. All fields are passed in;
// return the pointer to the new node to caller
/*****************************************************************/
NodePtr Make( NodeClass Class, char Symbol, int value,
					 NodePtr Left, NodePtr Right )
{ // Make
	NodePtr Node = (NodePtr)malloc( sizeof( struct NodeType ) );
//	ASSERT( ... node’s space is really there ... );
	Node->Class  = Class;
	Node->Symbol = Symbol;
	Node->LitVal = value;
	Node->Left   = Left;
	Node->Right  = Right;
	return Node;
} //end Make

/*****************************************************************/
// Function to copy a node.
// recursively copy tree pointed to by Root.
// Pass pointer to the copy to its caller
/*****************************************************************/
NodePtr Copy( NodePtr Root )
{ // Copy
	if ( Root == NULL ) {
			return NULL;
	}else{
			return Make( Root->Class, Root->Symbol,
				Root->LitVal,
				Copy( Root->Left ),
				Copy( Root->Right )
			);
	} //end if
} //end Copy

/*****************************************************************/
// Function to print a tree.
/*****************************************************************/
void PrintTree( NodePtr Root )
{ // PrintTree
   if ( Root != NULL ) {
			if ( Root->Class == Operator ) {
				printf( "(" );
				outfile << "(";
			} //end if
       PrintTree( Root->Left );
			if ( Root->Class == Literal ) {
				printf( "%d", Root->LitVal ); // prints ints > 9
				outfile << Root->LitVal; // prints ints > 9
			}else{
				printf( "%c", Root->Symbol );
				outfile << Root->Symbol;
			} //end if
			PrintTree( Root->Right );
			if ( Root->Class == Operator ) {
				printf( ")" );
				outfile << ")";
			} //end if
   } //end if
} //end PrintTree

/*****************************************************************/
// Function to build a tree.
/*****************************************************************/
NodePtr Primary( )
{ // Primary
   //NextChar = getchar();
   char Symbol = NextChar;		// first_set = { '(', ‘&’, IDENT, LIT }
   NodePtr Temp;

   GetNextChar();				// skip over current Symbol
   if ( isdigit( Symbol ) ) {
        return Make( Literal, Symbol, (int)(Symbol-'0'), NULL, NULL ); // end node: don’t recurse
   } else if ( isalpha( Symbol ) ) {
     	return Make( Identifier, tolower( Symbol ), 0, NULL, NULL ); // also end node: don’t recurse
   } else if ( '(' == Symbol ) {
     	Temp = Expression();
     	//Must_Be( ')' );
     	if ( NextChar != ')') {
            printf( ") expected, not found.\n" );
			outfile << ") expected, not found.\n";
            exit(0);
     	} else {
     	    GetNextChar();
            return Temp;
        }

	} else if ( Symbol == '&' ) {
	    return Make(  Operator, '&', 0, NULL, Primary() );
	} else {
     	printf( "\nIllegal character '%c'.", Symbol );
		outfile << "\nIllegal character " << Symbol << ".";
        return NULL;
	} //end if
	// impossible to reach! Hence check Herb!!
} //end Primary

/*****************************************************************/
// Function to build a tree factor with left-associativity.
// exponentiation operator '^' left-associatively
/*****************************************************************/
NodePtr Factor()
	{ // Factor
	NodePtr Left = Primary();
	//NextChar = getchar();
	while ( NextChar == '^' ) {
			GetNextChar(); 		// skip over '^'
     	Left = Make( Operator, '^', 0, Left, Primary() );
   } //end while
   return Left;
} //end Factor


/*****************************************************************/
// Function to build a tree factor with right-associativity.
// exponentiation operator '^' right-associative
/*****************************************************************/
/*
NodePtr Factor()
	{ // Factor
	NodePtr Left = Primary();
	if ( NextChar == '^' ) {
			GetNextChar(); 		// skip over '^'
     	Left = Make( Operator, '^', 0, Left, Factor() );
   } //end if
   return Left;
} //end Factor
*/

/*****************************************************************/
// Function to build a tree term.
// multiply operators ‘*’ and ‘/’, later add ‘%’
/*****************************************************************/
NodePtr Term( )
	{ // Term
	char Op;						// remember ‘*’ or ‘/’
	NodePtr Left = Factor();
	//NextChar = getchar();
	while ( NextChar == '*' || NextChar == '/' ) {
     	Op = NextChar;				// remember ‘*’ or ‘/’
     	GetNextChar(); 				// skip over Op

     	Left = Make( Operator, Op, 0, Left, Factor() ); // note 0 below for LitVal is just a dummy
    } //end while
    return Left;
} //end Term

/*****************************************************************/
// Function to build a tree expression.
// parse expression and build tree
// using Term() and higher priority functions/ops
// all returning pointers to nodes
// in Expression() handle ‘+’ and ‘-’ operators
/*****************************************************************/
NodePtr Expression()
	{ // Expression
	char Op;						// remember ‘+’ or ‘-’
	NodePtr Left = Term();			// handle all higher prior.
	//NextChar = getchar();
	while ( ( NextChar == '+' ) || ( NextChar == '-' ) ) {
			Op = NextChar;			// remember ‘+’ or ‘-’
            GetNextChar();			// skip Op
			// note 0 below for LitVal is just a dummy
			Left = Make( Operator, Op, 0, Left, Term() );
	} //end while
	return Left;
} //end Expression

/*****************************************************************/
// Function to derive.
/*****************************************************************/
NodePtr Derive( NodePtr Root )
{ // Derive
    NodePtr OneNode = Make( Literal, '1', 1, NULL, NULL );
	if ( NULL == Root ) {
			return NULL;
	}else{
			switch ( Root->Class ) {
		    case Literal:
				return Make( Literal, '0', 0, NULL, NULL );
		    case Identifier:
				if ( ( Root->Symbol == 'x' ) || ( Root->Symbol == 'X' ) ) {
			    	return Make( Literal, '1', 1, NULL, NULL );
				}else{
			    	return Make( Literal, '0', 0, NULL, NULL );
				} //end if
			case Operator:
                switch ( Root->Symbol ) {
                        case '+': case '-':
                            return Make( Operator, Root->Symbol, 0,
                                Derive( Root->Left ), Derive( Root->Right ) );
                        case '*':
                            return Make( Operator, '+', 0,
                                Make( Operator, '*', 0, Derive( Root->Left ),
                                        Copy( Root->Right ) ),
                                Make( Operator, '*', 0, Copy( Root->Left ),
                                        Derive( Root->Right ) ) );
                        case '/':
                            return Make( Operator, '/', 0,
                                Make( Operator, '-', 0,
                                Make( Operator, '*', 0, Derive( Root->Left ),
                                        Copy( Root->Right ) ),
                                Make( Operator, '*', 0, Copy( Root->Left ),
                                        Derive( Root->Right ) ) ),
                                Make( Operator, '*', 0,
                                        Copy( Root->Right ), Copy( Root->Right ) ) );
                        case '^':
                            return Make( Operator, '+', 0,
                                Make( Operator, '*', 0,
                                Derive( Root->Left ),
                                Make( Operator, '*', 0,
                                Copy( Root->Right ),
                                Make( Operator, '^', 0,
                                Copy( Root->Left ),
                                Make( Operator, '-', 0,
                                Copy( Root->Right ),
                                //Copy( & OneNode ) ) ) ) ),
                                Copy( OneNode ) ) ) ) ),
                                Make( Operator, '*', 0,
                                Make( Operator, '*', 0,
                                Make( Operator, '&', 0,
                                NULL,
                                Copy( Root->Left ) ),
                                Derive( Root->Right ) ),
                                Make( Operator, '^', 0,
                                Copy( Root->Left ),
                                Copy( Root->Right ) ) ) );
                        case '&':
                            if ( Root->Left != NULL ) {
                                printf( "ln has only one operand.\n" );
								outfile << "ln has only one operand.\n";
                            } //end if
                } //end switch
			} //end switch
    } //end if
} //end Derive

/*****************************************************************/
// Function to simplify.
/*****************************************************************/
NodePtr Simplify( NodePtr Root )
{ // Simplify
    NodePtr NullNode = Make( Literal, '0', 0, NULL, NULL );
    NodePtr OneNode = Make( Literal, '1', 1, NULL, NULL );
	int val = 0;			// accumulate integer values from + - * etc.
	if ( !Root ) {
		  return Root;
	}else{
       switch ( Root->Class ) {
      	case Literal:
      	case Identifier:
				return Root;
      	case Operator:
       		Root->Left  = Simplify( Root->Left );
       		Root->Right = Simplify( Root->Right );
       		switch ( Root->Symbol ) {
        			case '+':
         			if ( IsLit( '0', Root->Left ) ) {
							return Root->Right;
         			}else if ( IsLit( '0', Root->Right ) ) {
							return Root->Left;
						}else if ( BothLit( Root->Left, Root->Right ) ) { //cee extra credit code
							val = Root->Left->LitVal + Root->Right->LitVal;
							return Make( Literal, (char)( val + '0' ), val,
								NULL, NULL );
						}else{
							return Root; // no other simplifiction for ‘+’
						} //end if
                    case '-':
                      if ( IsLit( '0', Root->Right ) ) {
                        return Root->Left;
                      }else if ( BothLit( Root->Left, Root->Right ) ) { //cee extra credit code
                        val = Root->Left->LitVal - Root->Right->LitVal;
                        return Make( Literal, (char)( val + '0' ), val, NULL, NULL );
                      }else if ( IsEqual( Root->Left, Root->Right ) ) {
                        //return & NullNode;
                        return NullNode;
                      }else{
                        return Root;
                      } //end if
                    case '*':
                      if ( IsLit( '1', Root->Left ) ) {
                        return Root->Right;
                      }else if ( IsLit( '1', Root->Right ) ) {
                        return Root->Left;
                      }else if ( IsLit( '0', Root->Left ) || IsLit( '0', Root->Right ) ) {
                        //return & NullNode;
                        return NullNode;
                      }	else if ( BothLit( Root->Left, Root->Right ) ) { //cee extra credit code
							val = Root->Left->LitVal * Root->Right->LitVal;
							return Make( Literal, (char)( val + '0' ), val,
								NULL, NULL );
						}else{
							return Root; // no other simplifiction for ‘*’
						} //end if
                    case '/':
                       if ( IsLit( '1', Root->Right ) ) {
                         return Root->Left;
                       }else if ( IsLit( '0', Root->Left ) ) {
                         //return & NullNode;
                         return NullNode;
                       }else if ( IsEqual( Root->Left, Root->Right ) ) {
                         return OneNode;
                         //return Root->Left;
                       }else if ( BothLit( Root->Left, Root->Right ) ) { //cee extra credit code
							val = Root->Left->LitVal / Root->Right->LitVal;
							return Make( Literal, (char)( val + '0' ), val,
								NULL, NULL );
						}else{
							return Root; // no other simplifiction for ‘/’
						} //end if
                      case '^':
                        if ( IsLit( '0', Root->Right ) ) {			// x^0 = 1
                          return OneNode;
                          //return Root->Left;
                        }else if ( IsLit( '1', Root->Right ) ) {		// x^1 = x
                          return Root->Left;
                        }else if ( IsLit( '1', Root->Left ) ) {		// 1^x = 1
                          return OneNode;
                          //return Root->Left;
                        }else if ( BothLit( Root->Left, Root->Right ) ) { //cee extra credit code
							val = pow ( Root->Left->LitVal , Root->Right->LitVal );
							return Make( Literal, (char)( val + '0' ), val,
								NULL, NULL );
						}else{
							return Root; // no other simplifiction for ‘^’
						} //end if
						case '&':
						  return Root;
       		} //end switch
       } //end switch
	} //end if
} //end Simplify

/*****************************************************************/
// Function to simplify literals.
/*****************************************************************/
/*
NodePtr cee( NodePtr Root )
{
	if ( !Root ) {
		  return Root;
	}else{
	    if ( ( Root->Class == Operator ) && ( Root->Left->Class == Literal ) && ( Root->Right->Class == Literal ) ) {
       		switch ( Root->Symbol ) {
        			case '+':
                        Root->Left->LitVal = Root->Left->LitVal + Root->Right->LitVal;
                        Root->Right == NULL;
                        return Root->Left;
        			case '-':
                        Root->Left->LitVal = Root->Left->LitVal - Root->Right->LitVal;
                        Root->Right == NULL;
                        return Root->Left;
        			case '*':
                        Root->Left->LitVal = Root->Left->LitVal * Root->Right->LitVal;
                        Root->Right == NULL;
                        return Root->Left;
                    case '/':
                        Root->Left->LitVal = Root->Left->LitVal / Root->Right->LitVal;
                        Root->Right == NULL;
                        return Root->Left;
                    default:
                        return Root;
       		} //end switch
	    } //end if
    }
}
*/

/*****************************************************************/
// Function to compare two trees.
// return true only if both subtrees left and right are equal.
/*****************************************************************/
bool IsEqual( NodePtr Left, NodePtr Right )
{ // IsEqual
  if ( ( !Left ) && ( !Right ) ) {
    return true;
  }else if ( NULL == Left ) {
    // Right is known to be not NULL
    return false;
  }else if ( NULL == Right ) {
    // Left is known to be NOT NULL
    return false;
  }else if ( ( Left->Class == Literal ) && ( Right->Class == Literal ) ) {
    return ( Left->LitVal ) == ( Right->LitVal );
  }else if ( ( Left->Class == Identifier ) && ( Right->Class == Identifier )){
    return ( Left->Symbol ) == ( Right->Symbol );
  }else{
    // must be Operator; same?
    if ( ( Left->Symbol ) == ( Right->Symbol ) ) {
      // IsEqual yields true, only if both subtrees are equal
      return ( IsEqual( Left->Left, Right->Left ) &&
        IsEqual( Left->Right, Right->Right ) ) ||
 //       ( is_associative( Left->Symbol ) &&
			( IsEqual( Left->Left,  Right->Right ) &&
			 IsEqual( Left->Right, Right->Left ) );
    }else{
      return false;
    } //end if
  } //end if
  printf( "Impossible to reach in IsEqual.\n" );
  outfile << "Impossible to reach in IsEqual.\n";
} //end IsEqual

/*****************************************************************/
// Function to compare the symbol of a node with a character.
// return true only if both the values are equal.
/*****************************************************************/
bool IsLit( char sym, NodePtr node )
{ // IsLit
    if ( ( node->Class == Literal ) && ( node->Symbol == sym ) ) {
        return true;
    } else {
        return false;
    }
}

/*****************************************************************/
// Function to compare the literal value of two nodes.
// return true only if both the values are equal.
/*****************************************************************/
bool BothLit( NodePtr node1, NodePtr node2 )
{ // BothLit
    if ( node1->Class == Literal && node2->Class == Literal  ) {
        return true;
    } else {
        return false;
    }
}

/*****************************************************************/
// Main function.
/*****************************************************************/
int main ()
{ // main: Differentiation
	NodePtr root = NULL;
	outfile.open( OUTPUTFILE );
	printf("Enter f(x), ended with $: ");
	outfile << "Enter f(x), ended with $: ";
 	//Initialize();
    GetNextChar();
	root = Expression();

	if ( NextChar != '$' ) {
        printf( "\n$ expected, not found %c", NextChar );
        outfile << "\n$ expected, not found " << NextChar;
        exit(0);
	}

	printf( "\noriginal  f(x) = " );
	outfile << "\noriginal  f(x) = ";
	PrintTree( root );

	printf( "\nreduced   f(x) = " );
	outfile << "\nreduced   f(x) = ";
	root = Simplify( root );
	PrintTree( root );

    printf( "\nderived  f'(x) = " );
    outfile << "\nderived  f'(x) = ";
	root = Derive( root );
	PrintTree( root );

    printf( "\nreduced  f'(x) = " );
    outfile << "\nreduced  f'(x) = ";
 	root = Simplify( root );
	PrintTree( root );
/*
	root = cee( root );
	PrintTree( root );
*/
    outfile.close();
	return 0;
} //end main: Differentiation
