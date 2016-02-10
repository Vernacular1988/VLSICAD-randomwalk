
// Implementations of the circuit class
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <iostream>

#include "circuit.h"
#include "util.h"

#define TRACE_LOADING 1
#define TRACE_WALKING 0

Circuit::~Circuit() {
}

node Circuit::FindNode(const char * name) {
  if ( NodeExists(name) == 0 ) {
    return _nodes[name];
  } else {    
    printf("FATAL: node %s doesn't exist. Aborted...\n", name );
    exit (-1);
  }
}

node Circuit::FindOrCreateNode(const char * name) {
  node n;

  if ( NodeExists(name) == 0 ) {
    return _nodes[name];
  } else {    
    n = this->new_node();
    char *nn = strdup(name);
    _nodes[nn] = n;
    _loads[n] = 0.0;
    _dcaps[n] = 0.0;
    _supps[n] = 0.0;
#if TRACE_LOADING
    printf("Inserted %s, node list now has %lu elements\n", name, _nodes.size());
#endif
    return n;
  }
}

void Circuit::InsertDCAP(char *n_name, double val) {
  assert ( val>=0.0);
  node n = FindOrCreateNode(n_name);
  _dcaps[n] = val;
}

void Circuit::InsertLOAD(char *n_name, double val) {
  assert ( val>=0.0);
  node n = FindOrCreateNode(n_name);
  _loads[n] = val;
}

void Circuit::InsertSUPP(char *n_name, double val) {
  assert ( val>=0.0);
  node n = FindOrCreateNode(n_name);
  _supps[n] = val;
}

void Circuit::InsertWIRE(char *n_from, char *n_to, double r_val, double l_val) {
  node from, to;

  assert ( r_val >=0 && l_val >= 0 );
  assert ( !(r_val==0 && l_val==0) );

  to   = FindOrCreateNode(n_to);
  from = FindOrCreateNode(n_from);
  edge e = new_edge(from, to);
  WireProperties wp = WireProperties(r_val, l_val);
  _wires[e] = wp;
}

void Circuit::InsertSOLV(char *n_name) {
  FindOrCreateNode(n_name);
  char *nn = strdup(n_name);
  _what.push_back(nn);
}

int Circuit::LoadCircuit( FILE *fp ) {
  char   buffer[128], *chptr;
  DType  dt;
  char   from[64], to[64];
  double val1, val2;

  assert ( fp != NULL );
  while ( (chptr = fgets(buffer, sizeof(buffer)-1, fp)) != NULL ) {
    SkipBlanks(chptr);
    if ( strlen(buffer) > 1 ) {
      Tokenizer(buffer, &dt, &from[0], &to[0], val1, val2);

      switch (dt) {
      case WIRE:
	InsertWIRE(from, to, val1, val2);
#if TRACE_LOADING
	printf("WIRE inserted, %s->%s r=%f l=%f\n", from, to,val1, val2);
#endif

	break;
      case DCAP:
	InsertDCAP(from, val1);
#if TRACE_LOADING
	printf("DCAP inserted, %s %f\n", from,val1);
#endif
	break;

      case LOAD:
	InsertLOAD(from, val1);
#if TRACE_LOADING
	printf("LOAD inserted, %s %f\n", from, val1);
#endif
	break;

      case SUPP:
	InsertSUPP(from, val1);
#if TRACE_LOADING
	printf("SUPP inserted, %s %f\n", from, val1);
#endif
	break;

      case SOLV:
	InsertSOLV(from);
	break;

      default: // ignore the rest
	break;
      }
    }
  }
  fclose(fp);

  // make sure the graph is connected
  if ( is_connected()==false ) {
    printf("FATAL: the circuit is not connected. Exit...\n");
    return -1;
  }

  return 0;
}

// simple iterator
void Circuit::GetAllVoltages(int howmany) {
  for (unsigned int i=0; i<_what.size(); i++) {
    printf("V(%s) = %.3g\n", _what[i], GetVoltage(_what[i], howmany) );
  }
}

// algorithm to calculate the voltages
double Circuit::GetVoltage(const char *name, int howmany) 
{
  extern double dbl_rand();
  extern bool is_dbl_zero(double);

  if (  NodeExists(name) != 0 ) 
  {
    printf("Unable to locate node %s. Skipped.\n", name);
    return (0.0);
  }

  // do the calculation -- to be implemented
  //SUPP is answer if it's not 0


	
	node current, next;
	current=FindNode(name);

	double s=NodeSUPP(current); 
	double V_sum=0; //sum of voltage in each run
	double V, L; 
	double g,g_sum;
	double r;
	int i,j;

	node::inout_edges_iterator IE,BE,EE;


	//cout<<"initialization"<<endl;
	//cout<<NodeDCAP(current)<<endl;

	if (s>0)
		return ( s );
	else
	{
		for (i=0;i<howmany;i++)
		{
			V=0.0;
			current=FindNode(name);
			//cout<<"i="<<i<<endl;
			j=0;
			
			while (  !(NodeSUPP(current)>0) )
			{
				j++;
				L=NodeLOAD(current);
				//cout<<"L="<<L<<endl;
				
				g=0;

				g_sum=0;

				//Calculate the motel price
				
				BE= current.inout_edges_begin(); //begin

				EE= current.inout_edges_end(); //end
				

				for (IE=BE;IE!=EE;IE++)
				{

					g_sum=g_sum+(1/WireRES(*IE));

				}

				//cout<<"g_sum="<<g_sum<<endl;
				// IR-drop
				
				V=V-(L / g_sum);
				//cout<<"V="<<V<<endl;

				//random number
				r=dbl_rand();
				//cout<<"r="<<r<<endl;
				//walk

				for (IE=BE;IE!=EE;IE++)
				{
					g=(1/WireRES(*IE));

					r=r-(g/g_sum);

					if (r<0)
					{
						next=current.opposite(*IE);
						break;
					}
				}
				
			
				current=next;

			}
			//cout<<"j="<<j<<endl;
			V=V+NodeSUPP(current);
			//cout<<"V="<<V<<endl;


			V_sum=V_sum+V;
		}
		//cout<<"i="<<i<<endl;
		

	return ( V_sum/howmany );
	}





}








// end



