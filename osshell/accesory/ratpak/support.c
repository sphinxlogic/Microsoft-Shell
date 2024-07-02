//----------------------------------------------------------------------------
//  Package Title  ratpak
//  File           support.c
//  Author         Timothy David Corrie Jr. (timc@microsoft.com)
//  Copyright      (C) 1995-96 Microsoft
//  Date           10-21-96
//
//
//  Description
//
//     Contains support functions for rationals and numbers.
//
//  Special Information
//
//
//
//----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined( DOS )
#include <dosstub.h>
#else
#include <windows.h>
#endif
#include <ratpak.h>



BOOL fhalt;

LINKEDLIST gllfact;

void _readconstants( void );

#if defined( GEN_CONST )
void _dumprawrat( TCHAR *varname, PRAT rat );
void _dumprawnum( PNUMBER num );

static cbitsofprecision = 0;
#define READRAWRAT(v)
#define READRAWNUM(v)
#define DUMPRAWRAT(v) _dumprawrat(#v,v)
#define DUMPRAWNUM(v) fprintf( stderr, \
"// Autogenerated by _dumprawrat in support.c\n" ); \
    fprintf( stderr, "NUMBER init_" #v "= {\n" ); \
    _dumprawnum(v); \
    fprintf( stderr, "};\n" )


#else

#define DUMPRAWRAT(v)
#define DUMPRAWNUM(v)
#define READRAWRAT(v) createrat(v); DUPNUM(v->pp,(&(init_p_##v))); \
DUPNUM(v->pq,(&(init_q_##v)));
#define READRAWNUM(v) DUPNUM(v,(&(init_##v)))

#define RATIO_FOR_DECIMAL 9
#define DECIMAL 10
#define CALC_DECIMAL_DIGITS_DEFAULT 32

static cbitsofprecision = RATIO_FOR_DECIMAL * DECIMAL * 
                            CALC_DECIMAL_DIGITS_DEFAULT;

#include <ratconst.h>

#endif

unsigned char ftrueinfinite = FALSE;    // Set to true if you don't want 
                                        // chopping internally
long maxout;                            // precision used internally

PNUMBER num_one=NULL;
PNUMBER num_two=NULL;
PNUMBER num_five=NULL;
PNUMBER num_six=NULL;
PNUMBER num_ten=NULL;
PNUMBER num_nRadix=NULL;


PRAT ln_ten=NULL;
PRAT ln_two=NULL;
PRAT rat_zero=NULL;
PRAT rat_one=NULL;
PRAT rat_neg_one=NULL;
PRAT rat_two=NULL;
PRAT rat_six=NULL;
PRAT rat_half=NULL;
PRAT rat_ten=NULL;
PRAT pt_eight_five=NULL;
PRAT pi=NULL;
PRAT pi_over_two=NULL;
PRAT two_pi=NULL;
PRAT one_pt_five_pi=NULL;
PRAT e_to_one_half=NULL;
PRAT rat_exp=NULL;
PRAT rad_to_deg=NULL;
PRAT rad_to_grad=NULL;
PRAT rat_qword=NULL;
PRAT rat_dword=NULL;
PRAT rat_word=NULL;
PRAT rat_byte=NULL;
PRAT rat_360=NULL;
PRAT rat_400=NULL;
PRAT rat_180=NULL;
PRAT rat_200=NULL;
PRAT rat_nRadix=NULL;
PRAT rat_smallest=NULL;
PRAT rat_negsmallest=NULL;
PRAT rat_max_exp=NULL;
PRAT rat_min_exp=NULL;
PRAT rat_min_long=NULL;


//----------------------------------------------------------------------------
//
//  FUNCTION: ChangeRadix
//
//  ARGUMENTS:  base changing to, and precision to use.
//
//  RETURN: None
//
//  SIDE EFFECTS: sets a mess of constants.
//
//
//----------------------------------------------------------------------------

void changeRadix( long nRadix )

{
    ChangeConstants( nRadix, maxout );
}


//----------------------------------------------------------------------------
//
//  FUNCTION: changePrecision
//
//  ARGUMENTS:  Precision to use
//
//  RETURN: None
//
//  SIDE EFFECTS: sets a mess of constants.
//
//
//----------------------------------------------------------------------------

void changePrecision( long nPrecision )

{
    ChangeConstants( nRadix, nPrecision );
}

//----------------------------------------------------------------------------
//
//  FUNCTION: ChangeConstants
//
//  ARGUMENTS:  base changing to, and precision to use.
//
//  RETURN: None
//
//  SIDE EFFECTS: sets a mess of constants.
//
//
//----------------------------------------------------------------------------

void ChangeConstants( long nRadix, long nPrecision )

{
    long digit;
    DWORD dwLim;

    maxout = nPrecision;          
    fhalt = FALSE;
    
    
    // ratio is set to the number of digits in the current nRadix, you can get 
    // in the internal BASEX nRadix, this is important for length calculations 
    // in translating from nRadix to BASEX and back.                           

    dwLim = (DWORD)BASEX / (DWORD)nRadix;

    for ( digit = 1, ratio = 0; (DWORD)digit < dwLim; digit *= nRadix )
        {
        ratio++;
        }
    ratio += !ratio;

    destroynum(num_nRadix);
    num_nRadix=longtonum( nRadix, BASEX );
   
    destroyrat(rat_nRadix);
    rat_nRadix=longtorat( nRadix );
    
    // Check to see what we have to recalculate and what we don't
    if ( cbitsofprecision < ( ratio * nRadix * nPrecision ) )
        {
        ftrueinfinite=FALSE;
        num_one=longtonum( 1L, BASEX );
        DUMPRAWNUM(num_one);
        num_two=longtonum( 2L, BASEX );
        DUMPRAWNUM(num_two);
        num_five=longtonum( 5L, BASEX );
        DUMPRAWNUM(num_five);
        num_six=longtonum( 6L, BASEX );
        DUMPRAWNUM(num_six);
        num_ten=longtonum( 10L, BASEX );
        DUMPRAWNUM(num_ten);

        DUPRAT(rat_smallest,rat_nRadix);
        ratpowlong(&rat_smallest,-nPrecision);
        DUPRAT(rat_negsmallest,rat_smallest);
        rat_negsmallest->pp->sign = -1;
        DUMPRAWRAT(rat_smallest);
        DUMPRAWRAT(rat_negsmallest);
        createrat( rat_half );

        createrat( pt_eight_five );

        pt_eight_five->pp=longtonum( 85L, BASEX );
        pt_eight_five->pq=longtonum( 100L, BASEX );
        DUMPRAWRAT(pt_eight_five);

        rat_six = longtorat( 6L );
        DUMPRAWRAT(rat_six);

        rat_two=longtorat( 2L );
        DUMPRAWRAT(rat_two);

        rat_zero=longtorat( 0L );
        DUMPRAWRAT(rat_zero);

        rat_one=longtorat( 1L );
        DUMPRAWRAT(rat_one);

        rat_neg_one=longtorat( -1L );
        DUMPRAWRAT(rat_neg_one);

        DUPNUM(rat_half->pp,num_one);
        DUPNUM(rat_half->pq,num_two);
        DUMPRAWRAT(rat_half);

        rat_ten=longtorat( 10L );
        DUMPRAWRAT(rat_ten);
        
        // Apparently when dividing 180 by pi, another (internal) digit of 
        // precision is needed.
        maxout += ratio;
        DUPRAT(pi,rat_half);
        asinrat( &pi );
        mulrat( &pi, rat_six );
        DUMPRAWRAT(pi);
    
        DUPRAT(two_pi,pi);
        DUPRAT(pi_over_two,pi);
        DUPRAT(one_pt_five_pi,pi);
        addrat(&two_pi,pi);
        DUMPRAWRAT(two_pi);
    
        divrat(&pi_over_two,rat_two);
        DUMPRAWRAT(pi_over_two);
    
        addrat(&one_pt_five_pi,pi_over_two);
        DUMPRAWRAT(one_pt_five_pi);
    
        DUPRAT(e_to_one_half,rat_half);
        _exprat(&e_to_one_half);
        DUMPRAWRAT(e_to_one_half);

        DUPRAT(rat_exp,rat_one);
        _exprat(&rat_exp);
        DUMPRAWRAT(rat_exp);
        
        // WARNING: remember lograt uses exponent constants calculated above...

        DUPRAT(ln_ten,rat_ten);
        lograt( &ln_ten );
        DUMPRAWRAT(ln_ten);

        DUPRAT(ln_two,rat_two);
        lograt(&ln_two);
        DUMPRAWRAT(ln_two);
    
    
        destroyrat(rad_to_deg);
        rad_to_deg=longtorat(180L);
        divrat(&rad_to_deg,pi);
        DUMPRAWRAT(rad_to_deg);
    
        destroyrat(rad_to_grad);
        rad_to_grad=longtorat(200L);
        divrat(&rad_to_grad,pi);
        DUMPRAWRAT(rad_to_grad);
        maxout -= ratio;

        DUPRAT(rat_qword,rat_two);
        numpowlong( &(rat_qword->pp), 64, BASEX );
        subrat( &rat_qword, rat_one );
        DUMPRAWRAT(rat_qword);

        DUPRAT(rat_dword,rat_two);
        numpowlong( &(rat_dword->pp), 32, BASEX );
        subrat( &rat_dword, rat_one );
        DUMPRAWRAT(rat_dword);
        DUPRAT(rat_min_long,rat_dword);
        rat_min_long->pp->sign *= -1;
        DUMPRAWRAT(rat_min_long);

        rat_word = longtorat( 0xffff );
        DUMPRAWRAT(rat_word);
        rat_byte = longtorat( 0xff );
        DUMPRAWRAT(rat_byte);

        rat_400 = longtorat( 400 );
        DUMPRAWRAT(rat_400);

        rat_360 = longtorat( 360 );
        DUMPRAWRAT(rat_360);

        rat_200 = longtorat( 200 );
        DUMPRAWRAT(rat_200);

        rat_180 = longtorat( 180 );
        DUMPRAWRAT(rat_180);

        rat_max_exp = longtorat( 100000 );
        DUPRAT(rat_min_exp,rat_max_exp);
        rat_min_exp->pp->sign *= -1;
        DUMPRAWRAT(rat_max_exp);
        DUMPRAWRAT(rat_min_exp);

        cbitsofprecision = ratio * nRadix * nPrecision;
        }
    else
        {
        _readconstants();

        DUPRAT(rat_smallest,rat_nRadix);
        ratpowlong(&rat_smallest,-nPrecision);
        DUPRAT(rat_negsmallest,rat_smallest);
        rat_negsmallest->pp->sign = -1;
        }

}

//----------------------------------------------------------------------------
//
//  FUNCTION: intrat
//
//  ARGUMENTS:  pointer to x PRAT representation of number
//
//  RETURN: no return value x PRAT is smashed with integral number
//
//
//----------------------------------------------------------------------------

void intrat( PRAT *px)

{
    PRAT pret=NULL;
    PNUMBER pnum=NULL;
    TCHAR *psz;
    // Only do the intrat operation if number is nonzero.
    // and only if the bottom part is not one.
    if ( !zernum( (*px)->pp ) && !equnum( (*px)->pq, num_one ) )
        {
        psz=putrat( NULL, px, nRadix, FMT_FLOAT );
        pnum = innum( psz );
        zfree( psz );

        destroyrat( *px );
        *px = numtorat( pnum, nRadix );
        destroynum( pnum );

        DUPRAT(pret,*px);
        modrat( &pret, rat_one );
        
        subrat( px, pret );
        destroyrat( pret );
        }
}

//---------------------------------------------------------------------------
//
//  FUNCTION: rat_equ
//
//  ARGUMENTS:  PRAT a and PRAT b
//
//  RETURN: TRUE if equal FALSE otherwise.
//
//
//---------------------------------------------------------------------------

BOOL rat_equ( PRAT a, PRAT b )

{
    PRAT rattmp=NULL;
    BOOL bret;
    DUPRAT(rattmp,a);
    rattmp->pp->sign *= -1;
    addrat( &rattmp, b );
    bret = zernum( rattmp->pp );
    destroyrat( rattmp );
    return( bret );
}

//---------------------------------------------------------------------------
//
//  FUNCTION: rat_ge
//
//  ARGUMENTS:  PRAT a and PRAT b
//
//  RETURN: TRUE if a is greater than or equal to b
//
//
//---------------------------------------------------------------------------

BOOL rat_ge( PRAT a, PRAT b )

{
    PRAT rattmp=NULL;
    BOOL bret;
    DUPRAT(rattmp,a);
    b->pp->sign *= -1;
    addrat( &rattmp, b );
    b->pp->sign *= -1;
    bret = ( zernum( rattmp->pp ) || 
        rattmp->pp->sign * rattmp->pq->sign == 1 );
    destroyrat( rattmp );
    return( bret );
}


//---------------------------------------------------------------------------
//
//  FUNCTION: rat_gt
//
//  ARGUMENTS:  PRAT a and PRAT b
//
//  RETURN: TRUE if a is greater than b
//
//
//---------------------------------------------------------------------------

BOOL rat_gt( PRAT a, PRAT b )

{
    PRAT rattmp=NULL;
    BOOL bret;
    DUPRAT(rattmp,a);
    b->pp->sign *= -1;
    addrat( &rattmp, b );
    b->pp->sign *= -1;
    bret = ( !zernum( rattmp->pp ) && 
        rattmp->pp->sign * rattmp->pq->sign == 1 );
    destroyrat( rattmp );
    return( bret );
}

//---------------------------------------------------------------------------
//
//  FUNCTION: rat_le
//
//  ARGUMENTS:  PRAT a and PRAT b
//
//  RETURN: TRUE if a is less than or equal to b
//
//
//---------------------------------------------------------------------------

BOOL rat_le( PRAT a, PRAT b )

{

    PRAT rattmp=NULL;
    BOOL bret;
    DUPRAT(rattmp,a);
    b->pp->sign *= -1;
    addrat( &rattmp, b );
    b->pp->sign *= -1;
    bret = ( zernum( rattmp->pp ) || 
        rattmp->pp->sign * rattmp->pq->sign == -1 );
    destroyrat( rattmp );
    return( bret );
}


//---------------------------------------------------------------------------
//
//  FUNCTION: rat_lt
//
//  ARGUMENTS:  PRAT a and PRAT b
//
//  RETURN: TRUE if a is less than b
//
//
//---------------------------------------------------------------------------

BOOL rat_lt( PRAT a, PRAT b )

{
    PRAT rattmp=NULL;
    BOOL bret;
    DUPRAT(rattmp,a);
    b->pp->sign *= -1;
    addrat( &rattmp, b );
    b->pp->sign *= -1;
    bret = ( !zernum( rattmp->pp ) && 
        rattmp->pp->sign * rattmp->pq->sign == -1 );
    destroyrat( rattmp );
    return( bret );
}


//---------------------------------------------------------------------------
//
//  FUNCTION: rat_neq
//
//  ARGUMENTS:  PRAT a and PRAT b
//
//  RETURN: TRUE if a is not equal to b
//
//
//---------------------------------------------------------------------------


BOOL rat_neq( PRAT a, PRAT b )

{
    PRAT rattmp=NULL;
    BOOL bret;
    DUPRAT(rattmp,a);
    rattmp->pp->sign *= -1;
    addrat( &rattmp, b );
    bret = !( zernum( rattmp->pp ) );
    destroyrat( rattmp );
    return( bret );
}

//---------------------------------------------------------------------------
//
//  function: scale
//
//  ARGUMENTS:  pointer to x PRAT representation of number, and scaling factor
//
//  RETURN: no return, value x PRAT is smashed with a scaled number in the
//          range of the scalefact.
//
//---------------------------------------------------------------------------

void scale( PRAT *px, PRAT scalefact )

{
    long logscale;
    PRAT pret=NULL;
    DUPRAT(pret,*px);
    
    // Logscale is a quick way to tell how much extra precision is needed for 
    // scaleing by scalefact.
    logscale = ratio * ( (pret->pp->cdigit+pret->pp->exp) - 
            (pret->pq->cdigit+pret->pq->exp) );
    if ( logscale > 0 )
        {
        maxout += logscale;
        }
    else
        {
        logscale = 0;
        }

    divrat( &pret, scalefact);
    intrat(&pret);
    mulrat( &pret, scalefact);
    pret->pp->sign *= -1;
    addrat( px, pret);

    maxout -= logscale;
    destroyrat( pret );
}

//---------------------------------------------------------------------------
//
//  function: scale2pi
//
//  ARGUMENTS:  pointer to x PRAT representation of number
//
//  RETURN: no return, value x PRAT is smashed with a scaled number in the
//          range of 0..2pi
//
//---------------------------------------------------------------------------

void scale2pi( PRAT *px )

{
    long logscale;
    PRAT pret=NULL;
    PRAT my_two_pi=NULL;
    DUPRAT(pret,*px);
    
    // Logscale is a quick way to tell how much extra precision is needed for 
    // scaleing by 2 pi.
    logscale = ratio * ( (pret->pp->cdigit+pret->pp->exp) - 
            (pret->pq->cdigit+pret->pq->exp) );
    if ( logscale > 0 )
        {
        maxout += logscale;
        DUPRAT(my_two_pi,rat_half);
        asinrat( &my_two_pi );
        mulrat( &my_two_pi, rat_six );
        mulrat( &my_two_pi, rat_two );
        }
    else
        {
        DUPRAT(my_two_pi,two_pi);
        logscale = 0;
        }

    divrat( &pret, my_two_pi);
    intrat(&pret);
    mulrat( &pret, my_two_pi);
    pret->pp->sign *= -1;
    addrat( px, pret);

    maxout -= logscale;
    destroyrat( my_two_pi );
    destroyrat( pret );
}

//---------------------------------------------------------------------------
//
//  FUNCTION: inbetween
//
//  ARGUMENTS:  PRAT *px, and PRAT range.
//
//  RETURN: none, changes *px to -/+range, if px is outside -range..+range
//
//---------------------------------------------------------------------------

void inbetween( PRAT *px, PRAT range )

{
    if ( rat_gt(*px,range) )
        {
        DUPRAT(*px,range);
        }
    else
        {
        range->pp->sign *= -1;
        if ( rat_lt(*px,range) )
            {
            DUPRAT(*px,range);
            }
        range->pp->sign *= -1;
        }
}

#if defined( GEN_CONST )

//---------------------------------------------------------------------------
//
//  FUNCTION: _dumprawrat
//
//  ARGUMENTS:  char *name of variable, PRAT x
//
//  RETURN: none, prints the results of a dump of the internal structures
//          of a PRAT, suitable for READRAWRAT to stderr.
//
//---------------------------------------------------------------------------

void _dumprawrat( TCHAR *varname, PRAT rat )

{
    fprintf( stderr, "// Autogenerated by _dumprawrat in support.c\n" );
    fprintf( stderr, "NUMBER init_p_%s = {\n", varname );
    _dumprawnum( rat->pp );
    fprintf( stderr, "};\n" );
    fprintf( stderr, "NUMBER init_q_%s = {\n", varname );
    _dumprawnum( rat->pq );
    fprintf( stderr, "};\n" );
}

//---------------------------------------------------------------------------
//
//  FUNCTION: _dumprawnum
//
//  ARGUMENTS:  PNUMBER num
//
//  RETURN: none, prints the results of a dump of the internal structures
//          of a PNUMBER, suitable for READRAWNUM to stderr.
//
//---------------------------------------------------------------------------

void _dumprawnum( PNUMBER num )

{
    int i;

    fprintf( stderr, "\t%d,\n", num->sign );
    fprintf( stderr, "\t%d,\n", num->cdigit );
    fprintf( stderr, "\t%d,\n", num->exp );
    fprintf( stderr, "\t{ " );

    for ( i = 0; i < num->cdigit; i++ )
        {
        fprintf( stderr, " %d,", num->mant[i] );
        }
    fprintf( stderr, "}\n" );
}
#endif
void _readconstants( void )

{
    READRAWNUM(num_one);
    READRAWNUM(num_two);
    READRAWNUM(num_five);
    READRAWNUM(num_six);
    READRAWNUM(num_ten);
    READRAWRAT(pt_eight_five);
    READRAWRAT(rat_six);
    READRAWRAT(rat_two);
    READRAWRAT(rat_zero);
    READRAWRAT(rat_one);
    READRAWRAT(rat_neg_one);
    READRAWRAT(rat_half);
    READRAWRAT(rat_ten);
    READRAWRAT(pi);
    READRAWRAT(two_pi);
    READRAWRAT(pi_over_two);
    READRAWRAT(one_pt_five_pi);
    READRAWRAT(e_to_one_half);
    READRAWRAT(rat_exp);
    READRAWRAT(ln_ten);
    READRAWRAT(ln_two);
    READRAWRAT(rad_to_deg);
    READRAWRAT(rad_to_grad);
    READRAWRAT(rat_qword);
    READRAWRAT(rat_dword);
    READRAWRAT(rat_word);
    READRAWRAT(rat_byte);
    READRAWRAT(rat_360);
    READRAWRAT(rat_400);
    READRAWRAT(rat_180);
    READRAWRAT(rat_200);
    READRAWRAT(rat_smallest);
    READRAWRAT(rat_negsmallest);
    READRAWRAT(rat_max_exp);
    READRAWRAT(rat_min_exp);
    READRAWRAT(rat_min_long);
    DUPNUM(gllfact.pnum,num_one);
    gllfact.llprev = NULL;
    gllfact.llnext = NULL;
}

void factnum( IN OUT PLINKEDLIST *ppllfact, PNUMBER pnum )

{
    PNUMBER thisnum=NULL;
    PLINKEDLIST pllfact = *ppllfact;

    if ( pllfact->llnext == NULL ) 
        {
        // This factorial hasn't happened yet, lets compute it.
        DUPNUM(thisnum,pllfact->pnum);
        mulnumx(&thisnum,pnum);
        pllfact->llnext = (PLINKEDLIST)zmalloc( sizeof( LINKEDLIST ) );
        if (pllfact->llnext)
            {
            pllfact->llnext->pnum = thisnum;
            pllfact->llnext->llprev = pllfact;
            pllfact->llnext->llnext = NULL;
            }
        }
    *ppllfact = pllfact->llnext;
}


//---------------------------------------------------------------------------
//
//  FUNCTION: trimit
//
//  ARGUMENTS:  PRAT *px
//
//
//  DESCRIPTION: Chops off digits from rational numbers to avoid time 
//  explosions in calculations of functions using series.  
//  Gregory Stepanets proved it was enough to only keep the first n digits 
//  of the largest of p or q in the rational p over q form, and of course 
//  scale the smaller by the same number of digits.  This will give you 
//  n-1 digits of accuracy.  This dramatically speeds up calculations 
//  involving hundreds of digits or more.
//  The last part of this trim dealing with exponents never affects accuracy
//
//  RETURN: none, modifies the pointed to PRAT 
//
//---------------------------------------------------------------------------

void trimit( PRAT *px ) 

{
    if ( !ftrueinfinite )
        { 
        long trim;
        PNUMBER pp=(*px)->pp;
        PNUMBER pq=(*px)->pq;
        trim = ratio * (min((pp->cdigit+pp->exp),(pq->cdigit+pq->exp))-1) - maxout;
        if ( trim > ratio )
            {
            trim /= ratio;

            if ( trim <= pp->exp )
                {
                pp->exp -= trim;
                }
            else
                {
                memmove( MANT(pp), &(MANT(pp)[trim-pp->exp]), sizeof(MANTTYPE)*(pp->cdigit-trim+pp->exp) );
                pp->cdigit -= trim-pp->exp;
                pp->exp = 0;
                }

            if ( trim <= pq->exp )
                {
                pq->exp -= trim;
                }
            else
                {
                memmove( MANT(pq), &(MANT(pq)[trim-pq->exp]), sizeof(MANTTYPE)*(pq->cdigit-trim+pq->exp) );
                pq->cdigit -= trim-pq->exp;
                pq->exp = 0;
                }
            }
        trim = min(pp->exp,pq->exp);
        pp->exp -= trim;
        pq->exp -= trim;
        }
}
