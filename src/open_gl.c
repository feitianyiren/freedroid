/*
 *
 *   Copyright (c) 2007 Arthur Huillet
 *   Copyright (c) 2003 Johannes Prix
 *
 *
 *  This file is part of Freedroid
 *
 *  Freedroid is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Freedroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Freedroid; see the file COPYING. If not, write to the 
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 *
 */

/**
 * This file contains function relevant for OpenGL based graphics output.
 */

#define _open_gl_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "SDL_rotozoom.h"

// 28 degress is the magic angle for our iso view
#define COS_45 0.707106
#define SIN_45 0.707106
#define COS_28 0.88294759
#define SIN_28 0.46947156

/**
 * This is a wrapper for the SDL_Flip function, that will use either the
 * OpenGL buffer-swapping or the classic SDL flipper, depending on the
 * current output method, like OpenGL or not.
 */
int 
our_SDL_flip_wrapper ( )
{
    if ( use_open_gl )
	SDL_GL_SwapBuffers( ); 
    else
	return ( SDL_Flip ( Screen ) ) ;
    
    return ( 0 );
}; // int our_SDL_flip_wrapper ( SDL_Surface *screen )

/**
 * Here comes our SDL wrapper, that will either do a normal SDL blit or,
 * if OpenGL is present and enabled, use OpenGL to draw the scene.
 */
int
our_SDL_blit_surface_wrapper(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
#ifdef HAVE_LIBGL
    int bytes;
    int target_x, target_y ;
#endif
    
    if ( use_open_gl )
    {
#ifdef HAVE_LIBGL
	if ( src == NULL ) 
	{
	    DebugPrintf ( -4 , "\nNull source surface received. --> doing nothing." );
	    fflush ( stdout );
	    raise ( SIGSEGV );
	    return ( 0 );
	}
	
	if ( dst == Screen )
	{
	    
	    // DebugPrintf ( -1 , "\nReached our_SDL_blit_surface_wrapper." );
	    // fflush ( stdout );
	    
	    if ( dstrect == NULL )
		glRasterPos2f( 1 , GameConfig . screen_height - 1 );
	    else
	    {
		if ( dstrect -> x == 0 )
		    target_x = 1 ;
		else
		    target_x = dstrect -> x ;
		
		target_y = dstrect -> y + src -> h ;
		
		//--------------------
		// Here we add some extra security against raster positions (e.g. for
		// character screen and the like) SLIGHTLY out of bounds, which would
		// cause the entire blit to be canceled due to OpenGL internal policy.
		//
		if ( ( target_y >= 480 ) && ( target_y <= 482 ) ) target_y = 478 ; 
		
		glRasterPos2i( target_x , target_y ) ;
	    }
	    
	    if ( src -> w != 0 )
		bytes = src -> pitch / src -> w ;
	    else
	    {
		DebugPrintf ( -4 , "\nSurface of width 0 encountered. --> doing nothing." );
		fflush ( stdout );
		return ( 0 ) ;
	    }
	    
	    if ( srcrect != NULL )
	    {
		DebugPrintf ( -4 , "\nNon-Null source rect encountered. --> doing nothing." );
		fflush ( stdout );
		return ( 0 ) ;
	    }
	    
	    glDisable ( GL_TEXTURE_2D ); 

	    if ( bytes == 4 )
	    {
		glEnable( GL_ALPHA_TEST );  
		glDrawPixels( src -> w , src -> h, GL_BGRA , GL_UNSIGNED_BYTE , src -> pixels );		
		glDisable( GL_ALPHA_TEST );  

	    }
	    else if ( bytes == 3 )
	    {
		DebugPrintf ( 1 , "\nSurface has bytes: %d. " , bytes );
		fflush ( stdout );
		glDrawPixels( src -> w , src -> h, GL_RGB , GL_UNSIGNED_BYTE , src -> pixels );
	    }
	    else if ( bytes == 2 )
	    {
		DebugPrintf ( 1 , "\nSurface has bytes: %d. --> using GL_UNSIGNED_SHORT_5_6_5. " , bytes );
		fflush ( stdout );
		glDrawPixels( src -> w , src -> h, GL_RGB , GL_UNSIGNED_SHORT_5_6_5 , src -> pixels );
	    }
	    else
	    {
		DebugPrintf ( -4 , "\nSurface has bytes: %d.--> doing nothing. " , bytes );
		fflush ( stdout );
	    }
	    glEnable ( GL_TEXTURE_2D );  
	    return ( 0 ) ;
	}
	
	return SDL_BlitSurface ( src, srcrect, dst, dstrect);
#endif
    }
    else
    {
	return SDL_BlitSurface ( src, srcrect, dst, dstrect);
    }
    
    return 0 ;
    
}; // void our_SDL_blit_surface_wrapper(image, NULL, Screen, NULL)

/**
 *
 *
 */
void 
our_SDL_update_rect_wrapper ( SDL_Surface *screen, Sint32 x, Sint32 y, Sint32 w, Sint32 h ) 
{
  if ( use_open_gl )
    {
      our_SDL_flip_wrapper ( screen ) ;
    }
  else
    {
      SDL_UpdateRect ( screen, x, y, w, h ) ;
    }
}; // void our_SDL_update_rect_wrapper ( SDL_Surface *screen, Sint32 x, Sint32 y, Sint32 w, Sint32 h ) 

/**
 * This function will draw a rectangle to the screen.  It will use either
 * OpenGL or SDL for graphics output, depending on output method currently
 * set for the game.
 *
 * Note:  Rectangles in this sense have to be parallel to the screen
 *        coordinates.  Other rectangles can be drawn with the function
 *        blit_quad below (which only works with OpenGL output method).
 */
int 
our_SDL_fill_rect_wrapper (SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
#ifdef HAVE_LIBGL
  Uint8 r , g , b , a ;
#endif

  if ( use_open_gl )
    {
#ifdef HAVE_LIBGL
      if ( dst == Screen )
	{
	  glDisable( GL_TEXTURE_2D );
	  glEnable(GL_BLEND);

	  SDL_GetRGBA( color, Screen -> format , &r, &g, &b, &a);
	  glColor4ub( r , g , b , a );
	  if ( dstrect == NULL )
	    {
	      glBegin(GL_QUADS);
	      glVertex2i( 0                , GameConfig . screen_height );
	      glVertex2i( 0                ,   0 );
	      glVertex2i( 0 + GameConfig . screen_width ,   0 );
	      glVertex2i( 0 + GameConfig . screen_width , GameConfig . screen_height );
	      glEnd( );
	    }
	  else
	    {
	      glBegin(GL_QUADS);
	      glVertex2i( dstrect -> x                , dstrect -> y );
	      glVertex2i( dstrect -> x                , dstrect -> y + dstrect -> h );
	      glVertex2i( dstrect -> x + dstrect -> w , dstrect -> y + dstrect -> h );
	      glVertex2i( dstrect -> x + dstrect -> w , dstrect -> y );
	      glEnd( );
	    }
	  glEnable( GL_TEXTURE_2D );
	  glDisable(GL_BLEND);
	  return ( 0 );
	}
#endif
    }

  return ( SDL_FillRect ( dst, dstrect, color) ) ;

}; // int our_SDL_fill_rect_wrapper (SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)



/**
 * This function will draw quads, that are not nescessarily parallel to 
 * the screen coordinates to the screen.  It will currently only do 
 * something in OpenGL output method.
 */
int 
blit_quad ( int x1 , int y1 , int x2, int y2, int x3, int y3, int x4 , int y4 , Uint32 color )
{
#ifdef HAVE_LIBGL
  Uint8 r , g , b , a ;
#endif

  if ( use_open_gl )
    {
#ifdef HAVE_LIBGL
	glDisable( GL_TEXTURE_2D );
	
	SDL_GetRGBA( color, Screen -> format , &r, &g, &b, &a);
	glColor4ub( r , g , b , a );

	glBegin(GL_QUADS);
	glVertex2i( x1 , y1 );
	glVertex2i( x2 , y2 );
	glVertex2i( x3 , y3 );
	glVertex2i( x4 , y4 );
	glEnd( );

	glEnable( GL_TEXTURE_2D );
	return ( 0 );
#endif
    }

  return ( 0 );
}; // int blit_quad ( int x1 , int y1 , int x2, int y2, int x3, int y3, int x4 , int y4 , Uint32 color )


/**
 * Simon N.:  ISO functions.  these draw quads in the 3D planes
 */
void 
drawISOXYQuad ( int x , int y , int z , int w , int h ) 
{
#ifdef HAVE_LIBGL
    glVertex3f( x, y-h, z);
    glVertex3f( x, y, z);
    glVertex3f( x+ w * COS_28, y + w * SIN_28, z);
    glVertex3f( x+ w * COS_28, y-h + w * SIN_28, z);
#endif
}

void 
drawISOXZQuad(int x, int y, int z, int w, int d) 
{
#ifdef HAVE_LIBGL
    glVertex3f( x + d * COS_28 ,  y - d * SIN_28, z );
    glVertex3f(  x , y , z );
    glVertex3f(  x + w * COS_28 , y + w * SIN_28 , z );
    glVertex3f( x +w * COS_28 + d * COS_28, y + w * SIN_28 - d* SIN_28, z ) ;
#endif
}

void 
drawISOYZQuad(int x, int y, int z, int h, int d) 
{
#ifdef HAVE_LIBGL
	glVertex3f( x , y-h, z );
	glVertex3f( x, y, z );
	glVertex3f( x + d*COS_28 , y - d * SIN_28, z );
	glVertex3f( x + d*COS_28 , y -h - d * SIN_28, z  ) ;
#endif
}


/**
 * Simon N.: Draws an isometric energy bar.
 * dir : X_DIR | Y_DIR | Z_DIR
 * x,y,z : the position of the lower left hand corner
 * h : the height of the energy bar, as if viewed in the X direction
 * d : the depth of the energy bar, as if viewed in the X direction
 * fill : the percentage the energy bar is filled
 * c1 : the fill color
 * c1 : the background color
 */
void 
drawIsoEnergyBar(int dir, int x, int y, int z, int h, int d, int length, float fill, myColor *c1, myColor *c2  ) {
#ifdef HAVE_LIBGL
    int l = (int) (fill * length) ;
    int l2 = (int) length * (1.0-fill) ;
    int lcos, lsin, l2cos, l2sin ;
    glEnable(GL_BLEND) ;
    glColor4ub(c1->r, c1->g, c1->b, c1->a ) ;
    glDisable( GL_TEXTURE_2D );
    glBegin(GL_QUADS) ;

    // the rest of this is trig to work out the x,y,z co-ordinates of the quads
    if ( dir == X_DIR ) 
    {
	// we need to round these or sometimes the
	// quads will be out by 1 pixel
	lcos = (int) rint( l * COS_28) ;
	lsin = (int) rint( l * SIN_28) ;
	l2cos = (int) rint( l2 * COS_28) ;
	l2sin = (int) rint( l2 * SIN_28) ;
	
	drawISOXYQuad(x,y,z,l,h) ;
	drawISOYZQuad( x + lcos ,y + lsin ,z,h,d) ;
	drawISOXZQuad( x, y-h,z, l,d) ;
	glColor4ub(c2->r, c2->g, c2->b, c2->a ) ;
	drawISOXYQuad(x+ lcos,y+ lsin ,z,l2,h) ;
	drawISOYZQuad( x + l2cos + lcos ,y+ l2sin + lsin ,z,h,d) ;
	drawISOXZQuad( x + lcos, y+ lsin -h,z, l2,d) ;
	
    } 
    else if ( dir == Y_DIR) 
    {
	// this should be wcos, but we're saving variables :)
	lcos = (int) rint ( h * COS_28) ;
	lsin = (int) rint ( h * SIN_28) ;
	drawISOXYQuad(x,y,z,h,l) ;
	drawISOYZQuad( x + lcos ,y + lsin	,z,l,d) ;
	drawISOXZQuad( x, y-l,z, h,d) ;
	glColor4ub(c2->r, c2->g, c2->b, c2->a ) ;
	drawISOXYQuad(x,y-l,z,h,l2) ;
	drawISOYZQuad( x + lcos ,y -l + lsin ,z,l2,d) ;
	drawISOXZQuad( x, y-l-l2,z, h,d) ;
    } 
    else 
    {
	lcos = (int) rint( l * COS_28) ;
	lsin = (int) rint( l * SIN_28) ;
	// think of this a dcos, same reason above
	l2cos = (int) rint( d * COS_28) ;
	l2sin = (int) rint( d * SIN_28) ;
	drawISOXYQuad(x,y,z,d,h) ;
	drawISOYZQuad( x + l2cos ,y + l2sin ,z,h,l) ;
	drawISOXZQuad( x, y-h,z, d,l) ;
	
	glColor4ub(c2->r, c2->g, c2->b, c2->a ) ;
	drawISOYZQuad( x + l2cos + lcos ,y + l2sin - lsin ,z,h,l2) ;
	drawISOXZQuad( x+ lcos , y - lsin -h,z, d,l2) ;
    }
    glEnd() ;
    glEnable( GL_TEXTURE_2D );
    glDisable(GL_BLEND) ;

#endif
}; // void drawIsoEnergyBar(int dir, int x, int y, int z, int h, int d, int length, float fill, myColor *c1, myColor *c2  ) 


SDL_Surface*
our_SDL_display_format_wrapperAlpha ( SDL_Surface *surface )
{
    SDL_Surface* return_surface;

    if ( use_open_gl )
    {
	if ( surface == Screen ) return ( NULL );
	return_surface = SDL_DisplayFormatAlpha ( surface ) ;
	SDL_SetColorKey( return_surface , 0 , 0 ); // this should clear any color key in the dest surface
	SDL_SetAlpha( return_surface , SDL_SRCALPHA , SDL_ALPHA_OPAQUE );
	return ( return_surface ) ;
    }
    else
    {
	return ( SDL_DisplayFormatAlpha ( surface ) ) ; 
    }
    
    return ( NULL );
}; // SDL_Surface* our_SDL_display_format_wrapperAlpha ( SDL_Surface *surface )

/**
 * This function flips a given SDL_Surface.
 * 
 * This is particularly nescessary, since OpenGL has a different native
 * coordinate system than SDL and therefore images often appear flipped
 * around if one doesn't counter this effect with OpenGL by flipping the
 * images just once more in the same fashion.  That is what this function
 * does.
 */
void
flip_image_vertically ( SDL_Surface* tmp1 ) 
{
SDL_LockSurface(tmp1);

int nHH = tmp1->h >> 1;
int nPitch = tmp1->pitch;

unsigned char pBuf[nPitch + 1];
unsigned char * pSrc = (unsigned char *) tmp1->pixels;
unsigned char * pDst = (unsigned char *) tmp1->pixels + nPitch*(tmp1->h - 1);

while (nHH--)
    {
    memcpy(pBuf, pSrc, nPitch);
    memcpy(pSrc, pDst, nPitch);
    memcpy(pDst, pBuf, nPitch);

    pSrc += nPitch;
    pDst -= nPitch;
    }

SDL_UnlockSurface(tmp1);

}; // void flip_image_vertically ( SDL_Surface* tmp1 ) 

/**
 *
 *
 */
SDL_Surface* 
our_IMG_load_wrapper( const char *file )
{
    SDL_Surface* tmp1;
    
    if ( use_open_gl )
    {
	tmp1 = IMG_Load ( file ) ;
	
	if ( tmp1 == NULL ) return ( NULL );
	
	flip_image_vertically ( tmp1 ) ;
	
	return ( tmp1 ) ;
    }
    else
    {
	return ( IMG_Load ( file ) ) ;
    }
}; // SDL_Surface* our_IMG_load_wrapper( const char *file )

/**
 * There is need to do some padding, cause OpenGL textures need to have
 * a format: width and length both each a power of two.  Therefore some
 * extra alpha to the sides must be inserted.  This is what this function
 * is supposed to do:  manually adding hte proper amount of padding to
 * the surface, so that the dimensions will reach the next biggest power
 * of two in both directions, width and length.
 */
SDL_Surface*
pad_image_for_texture ( SDL_Surface* our_surface ) 
{
    int x = 1 ;
    int y = 1 ;
    SDL_Surface* padded_surf;
    SDL_Rect dest;

    
    while ( x < our_surface -> w ) x <<= 1;
    while ( y < our_surface -> h ) y <<= 1;

    padded_surf = SDL_CreateRGBSurface( 0 , x , y , 32, rmask, gmask, bmask, amask);

    SDL_SetAlpha( our_surface , 0 , 0 );
    SDL_SetColorKey( our_surface , 0 , 0x0FF );
    dest . x = 0;
    dest . y = y - our_surface -> h ;
    dest . w = our_surface -> w ;
    dest . h = our_surface -> h ;
                   
    our_SDL_blit_surface_wrapper ( our_surface, NULL , padded_surf , & dest );
    
    return ( padded_surf );

}; // SDL_Surface* pad_image_for_texture ( SDL_Surface* our_surface ) 

/**
 * If OpenGL is in use, we need to make textured quads out of our normal
 * SDL surfaces.
 * 
 * make_texture_out_of_surface and make_texture_out_of_prepadded_image are
 * the entry points for this function.
 */
static void
do_make_texture_out_of_surface ( iso_image* our_image, int txw, int txh, void * data) 
{

#ifdef HAVE_LIBGL

    glPixelStorei ( GL_UNPACK_ALIGNMENT , 1 );
    
    if ( our_image -> texture ) 
	if ( * our_image-> texture )
		{
		goto reuse_tex;
		}

    our_image -> texture = & ( all_freedroid_textures [ next_texture_index_to_use ] ) ;
    our_image -> texture_has_been_created = TRUE ;
    next_texture_index_to_use ++ ;
    if ( next_texture_index_to_use >= MAX_AMOUNT_OF_TEXTURES_WE_WILL_USE )
    {
	ErrorMessage ( __FUNCTION__  , 
				   "Ran out of initialized texture positions to use for new textures.",
				   PLEASE_INFORM, IS_FATAL );
    }
    else
    {
	DebugPrintf ( 1 , "\nTexture positions remaining: %d." , MAX_AMOUNT_OF_TEXTURES_WE_WILL_USE - next_texture_index_to_use );
    }
    
    reuse_tex:
    DebugPrintf ( 1 , "Using texture %d\n", *our_image->texture);
   
    glBindTexture( GL_TEXTURE_2D, * ( our_image -> texture ) );
    
    // We tend to scale those textures a lot, so we use linear filtering otherwise
    // the result is not so good.
    glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
    
    // Generate The Texture 
    glTexImage2D( GL_TEXTURE_2D, 0, 4, txw ,  txh , 0, GL_BGRA,  GL_UNSIGNED_BYTE, data);

    open_gl_check_error_status ( __FUNCTION__ ); 

#endif

}

void
make_texture_out_of_surface ( iso_image* our_image ) 
{

#ifdef HAVE_LIBGL

    SDL_Surface* right_sized_image ;

    //--------------------
    // This fills up the image with transparent material, so that 
    // it will have powers of 2 as the dimensions, which is a requirement
    // for textures on most OpenGL capable cards.
    //
    right_sized_image = pad_image_for_texture ( our_image -> surface ) ;
    our_image -> texture_width = right_sized_image -> w ;
    our_image -> texture_height = right_sized_image -> h ;
    our_image -> original_image_width = our_image -> surface -> w ;
    our_image -> original_image_height = our_image -> surface -> h ;
    
    //--------------------
    // Having prepared the raw image it's now time to create the real
    // textures.
    //
    do_make_texture_out_of_surface ( our_image, right_sized_image -> w, right_sized_image -> h, right_sized_image -> pixels );
    SDL_FreeSurface ( right_sized_image );
    
    //--------------------
    // Now that the texture has been created, we assume that the image is
    // not needed any more and can be freed now!  

    // So as to detect any occurances of access to the surface once the 
    // texture has been created, we set the surface to the NULL pointer to
    // ENCOURAGE SEGMENTATION FAULTS when doing this so as to eliminate
    // these occurances with the debugger...
    //
    SDL_FreeSurface ( our_image -> surface );
    our_image -> surface = NULL ;
    
#endif
    
}; // void make_texture_out_of_surface ( iso_image* our_image )

void make_texture_out_of_prepadded_image ( iso_image * our_image )
{
//the image is prepadded we have nothing to do    
    do_make_texture_out_of_surface ( our_image, our_image->surface->w, our_image->surface->h, our_image->surface->pixels);
    
}; // void make_texture_out_of_prepadded_image (...)


/**
 * This function checks the error status of the OpenGL driver.  An error
 * will produce at least a warning message, maybe even program termination
 * if the errors are really severe.
 */
void
open_gl_check_error_status ( const char* name_of_calling_function  )
{

#ifdef HAVE_LIBGL
    
    switch ( glGetError( ) )
    {
	case GL_NO_ERROR:
	    //--------------------
	    // All is well.  No messages need to be generated...
	    break;
	case GL_INVALID_ENUM:
	    fprintf ( stderr , "\nCheck occured in function: %s." , name_of_calling_function );
	    ErrorMessage ( __FUNCTION__  , 
				       "Error code GL_INVALID_ENUM received!", PLEASE_INFORM, IS_WARNING_ONLY );
	    break;
	case GL_INVALID_VALUE:
	    fprintf ( stderr , "\nCheck occured in function: %s." , name_of_calling_function );
	    ErrorMessage ( __FUNCTION__  , 
				       "Error code GL_INVALID_VALUE received!", PLEASE_INFORM, IS_WARNING_ONLY );
	    break;
	case GL_INVALID_OPERATION:
	    fprintf ( stderr , "\nCheck occured in function: %s." , name_of_calling_function );
	    ErrorMessage ( __FUNCTION__  , 
				       "Error code GL_INVALID_OPERATION received!", PLEASE_INFORM, IS_WARNING_ONLY );
	    break;
	case GL_STACK_OVERFLOW:
	    fprintf ( stderr , "\nCheck occured in function: %s." , name_of_calling_function );
	    ErrorMessage ( __FUNCTION__  , 
				       "Error code GL_STACK_OVERFLOW received!", PLEASE_INFORM, IS_FATAL );
	    break;
	case GL_STACK_UNDERFLOW:
	    fprintf ( stderr , "\nCheck occured in function: %s." , name_of_calling_function );
	    ErrorMessage ( __FUNCTION__  , 
				       "Error code GL_STACK_UNDERFLOW received!", PLEASE_INFORM, IS_FATAL );
	    break;
	case GL_OUT_OF_MEMORY:
	    fprintf ( stderr , "\nCheck occured in function: %s." , name_of_calling_function );
	    ErrorMessage ( __FUNCTION__  , 
				       "Error code GL_OUT_OF_MEMORY received!", PLEASE_INFORM, IS_FATAL );
	    // raise ( SIGSEGV );
	    break;
	default:
	    fprintf ( stderr , "\nCheck occured in function: %s." , name_of_calling_function );
	    ErrorMessage ( __FUNCTION__  , 
				       "Unhandled error code received!", PLEASE_INFORM, IS_FATAL );
	    break;
    }
#endif
}; // void open_gl_check_error_status ( char* name_of_calling_function  )

/**
 * Clear the screen, plain and simple, but only in OpenGL mode.
 */
void 
clear_screen( void ) {
#ifdef HAVE_LIBGL
	
    if ( use_open_gl )
    {
	//--------------------
	// Now we specify the default color to use when clearing the screen
	// with glClear.  Accoding to a hint from Derrick Pallas, this might
	// be causing problems with S3 graphics cards, so we move the color
	// specification out and away from initialisation part to the (only) 
	// location where glClear is actually used.
	//
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glClear(GL_COLOR_BUFFER_BIT);

    }

#endif

}; // void clear_screen ( void )

/**
 * This function does the first part of the OpenGL parameter 
 * initialisation.  We've made this chunk of code into a separate function
 * such that the frequent issues with OpenGL drivers can be attributed to
 * a particular spot in the code more easily.
 */
void
safely_set_open_gl_viewport_and_matrix_mode ( void )
{

#ifdef HAVE_LIBGL

    glViewport ( 0 , 0 , GameConfig . screen_width , GameConfig . screen_height );
    glMatrixMode ( GL_PROJECTION ) ;
    glLoadIdentity ( ) ;
    glOrtho( 0.0f , GameConfig . screen_width , GameConfig . screen_height , 0.0f , -1.0f , 1.0f );
    glMatrixMode ( GL_MODELVIEW );

    open_gl_check_error_status ( __FUNCTION__ );

#endif

}; // void safely_set_open_gl_viewport_and_matrix_mode ( void )

/**
 * This function does the second part of the OpenGL parameter 
 * initialisation.  We've made this chunk of code into a separate function
 * such that the frequent issues with OpenGL drivers can be attributed to
 * a particular spot in the code more easily.
 */
void
safely_set_some_open_gl_flags_and_shade_model ( void )
{

#ifdef HAVE_LIBGL

    glEnable ( GL_TEXTURE_2D );
	
    glShadeModel ( GL_FLAT );

    glDisable( GL_ALPHA_TEST );
    glAlphaFunc ( GL_GREATER , 0.499999 ) ;

    glDisable ( GL_BLEND );
    glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;

    glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    open_gl_check_error_status ( __FUNCTION__ );

#endif

}; // void safely_set_some_open_gl_flags_and_shade_model ( void )

/**
 * Initialize the OpenGL interface.
 */
int
safely_initialize_our_default_open_gl_parameters ( void )
{
#ifdef HAVE_LIBGL
    //--------------------
    // Set up the screne, viewport matrix, coordinate system and all that...
    //
    safely_set_open_gl_viewport_and_matrix_mode ();
    
    safely_set_some_open_gl_flags_and_shade_model ();
    
    glGenTextures( MAX_AMOUNT_OF_TEXTURES_WE_WILL_USE , & ( all_freedroid_textures [ 0 ] ) );  
    
    next_texture_index_to_use = 0 ;
    
    open_gl_check_error_status ( __FUNCTION__ );
    
#endif
    
    return( TRUE );

}; // int safely_initialize_our_default_open_gl_parameters ( void )


/* This function draws a textured quad on screen. */

#ifdef HAVE_LIBGL
static inline void 
draw_gl_textured_quad_helper ( int x0, int y0, int x1, int y1 ) 
{
  
    glBegin(GL_QUADS);
    glTexCoord2i( 0.0f, 1.0 ); 
    glVertex2i( x0, y0 );
    glTexCoord2i( 0.0f, 0.0 ); 
    glVertex2i( x0, y1 );
    glTexCoord2i( 1.0f, 0.0 ); 
    glVertex2i( x1, y1 );
    glTexCoord2f( 1.0f, 1.0 ); 
    glVertex2i( x1, y0 );
    glEnd( );

};
#endif


void
draw_gl_textured_quad_at_map_position ( iso_image * our_floor_iso_image , 
				       float our_col , float our_line , 
				       float r, float g , float b , 
				       int highlight_texture, int blend, float zoom_factor ) 
{

#ifdef HAVE_LIBGL
    float a = 1.0;
    int x, y;
    glEnable(GL_ALPHA_TEST);

    if ( (( blend == TRANSPARENCY_FOR_WALLS ) && GameConfig . transparency) || blend == TRANSPARENCY_CUROBJECT ) 
    {
	glEnable ( GL_BLEND ) ;
	a = 0.50;
    }
    else if ( blend == TRANSPARENCY_FOR_SEE_THROUGH_OBJECTS ) 
    {
	glEnable ( GL_BLEND ) ;
    }
    

  
    translate_map_point_to_screen_pixel ( our_col , our_line , &x, &y, zoom_factor ); 
    x +=  our_floor_iso_image -> offset_x * zoom_factor ; 
    y +=  our_floor_iso_image -> offset_y * zoom_factor ; 
    
    glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glColor4f( r , g , b, a );

    glBindTexture( GL_TEXTURE_2D, * ( our_floor_iso_image -> texture ) );
    
    draw_gl_textured_quad_helper ( x, y, x + our_floor_iso_image -> texture_width * zoom_factor,
				y + our_floor_iso_image -> texture_height * zoom_factor) ;

    if ( highlight_texture )
    {
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE , GL_ONE );

	//--------------------
	// Now we draw our quad AGAIN!
	//
    	draw_gl_textured_quad_helper ( x, y, x + our_floor_iso_image -> texture_width * zoom_factor,
				y + our_floor_iso_image -> texture_height * zoom_factor) ;
	glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );

    }

    glDisable( GL_BLEND );
    glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    glDisable(GL_ALPHA_TEST);

#endif

}; // void draw_gl_textured_quad_at_map_position ( ... )


/* ------------------------------------------------------------
 * Draw an open gl textured quad at the given screen position
 * */
void
draw_gl_textured_quad_at_screen_position ( iso_image * our_floor_iso_image , int x , int y ) 
{
#ifdef HAVE_LIBGL

	glEnable ( GL_BLEND );

	glBindTexture( GL_TEXTURE_2D, *our_floor_iso_image -> texture);
        draw_gl_textured_quad_helper ( x, y, x + our_floor_iso_image -> texture_width, 
			y + our_floor_iso_image -> texture_height);
	glDisable ( GL_BLEND );

#endif
}

/**
 *
 *
 */
void
draw_gl_scaled_textured_quad_at_screen_position ( iso_image * our_floor_iso_image , int x , int y , float scale_factor ) 
{

#ifdef HAVE_LIBGL
    
    glEnable ( GL_BLEND );

    glBindTexture( GL_TEXTURE_2D, * ( our_floor_iso_image -> texture ) );

    draw_gl_textured_quad_helper(x, y, x + our_floor_iso_image -> texture_width * scale_factor,
			y + our_floor_iso_image -> texture_height * scale_factor);

    glDisable ( GL_BLEND );
    
#endif
    
};

void
draw_gl_scaled_quad_from_atlas_at_screen_position ( iso_image * ourimg, gl_atlas_member * ouratl, int x, int y, float scale )
{
#ifdef HAVE_LIBGL
glBindTexture(GL_TEXTURE_2D, ouratl->tex);
glEnable(GL_ALPHA_TEST); 
glBegin(GL_QUADS);
glTexCoord2f( ouratl->x1, ouratl->y2);
glVertex2i( x, y );
glTexCoord2f( ouratl->x1, ouratl->y1 );
glVertex2i( x, y + ourimg->original_image_height * scale);
glTexCoord2f( ouratl->x2, ouratl->y1 );
glVertex2i( x + ourimg->original_image_width * scale, y + ourimg->original_image_height * scale);
glTexCoord2f(  ouratl->x2, ouratl->y2 );
glVertex2i( x  + ourimg->original_image_width * scale, y);
glEnd();
glDisable(GL_ALPHA_TEST);
#endif
}


/**
 * This function blits some texture to the screen, but instead of using
 * the usual 1:1 ratio, this function will instead stretch the texture
 * received such that the ratio corrsponds to the current (possibly wider)
 * screen dimension.
 */
void
draw_gl_bg_textured_quad_at_screen_position ( iso_image * our_floor_iso_image , int x , int y ) 
{

#ifdef HAVE_LIBGL
    int image_end_x;
    int image_end_y;

    glEnable ( GL_BLEND );

    if ( our_floor_iso_image -> texture_height == 1024 ) //then the image is 1024x768
	{ /*dirty hack for better scaling*/
        image_end_x = x + our_floor_iso_image -> texture_width * GameConfig . screen_width / 1024 ;
        image_end_y = y + our_floor_iso_image -> texture_height * GameConfig . screen_height / 768 ;
	}
    else
	{
        image_end_x = x + our_floor_iso_image -> texture_width * GameConfig . screen_width / 640 ;
        image_end_y = y + our_floor_iso_image -> texture_height * GameConfig . screen_height / 480 ;
	}

    glBindTexture(GL_TEXTURE_2D, *(our_floor_iso_image -> texture));
    draw_gl_textured_quad_helper ( x, y, image_end_x, 
			image_end_y);

    glDisable ( GL_BLEND );

#endif

}

/**
 *  draws a textured quad at the given screen position, after 
 * setting blend mode to additive
 */
void
blit_semitransparent_open_gl_texture_to_screen_position ( iso_image * our_floor_iso_image , int x , int y , float scale_factor ) 
{

#ifdef HAVE_LIBGL

    glBlendFunc ( GL_ONE, GL_ONE );
    draw_gl_scaled_textured_quad_at_screen_position ( our_floor_iso_image , x , y , scale_factor ) ;
    glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

#endif

}; // void blit_semitransparent_open_gl_texture_to_screen_position ( ... )

/**
 * This function restores the menu background, that must have been stored
 * before using the function of similar name.
 */
void
RestoreMenuBackground ( int backup_slot )
{
    if ( use_open_gl )
    {
#ifdef HAVE_LIBGL
    int h = (GameConfig . screen_height > 1024 ) ? 2048 : 1024;
    int w = (GameConfig . screen_width > 1024 ) ? 2048 : 1024;
    
    glBindTexture( GL_TEXTURE_2D, * ( StoredMenuBackgroundTex [ backup_slot ] ) );

    glBegin(GL_QUADS);
    glTexCoord2i( 0.0f, 1.0f );
    glVertex2i( 0 , 0 );
    glTexCoord2i( 0.0f, 0.0 );
    glVertex2i( 0 , h );
    glTexCoord2i( 1.0f, 0.0 );
    glVertex2i( w , h );
    glTexCoord2f( 1.0f, 1.0 );
    glVertex2i( w , 0 );
    glEnd( );

    glDisable ( GL_BLEND );

#endif
    }
    else
    {
	our_SDL_blit_surface_wrapper ( StoredMenuBackground [ backup_slot ] , NULL , Screen , NULL );
    }
}; // void RestoreMenuBackground ( void )

/**
 * This function stores the current background as the background for a
 * menu, so that it can be refreshed much faster than by reassembling it
 * every frame.
 */
void
StoreMenuBackground ( int backup_slot )
{
    static int first_call = TRUE ;

    if ( first_call )
    {
	StoredMenuBackground [ 0 ] = NULL ;
	StoredMenuBackground [ 1 ] = NULL ;
	first_call = FALSE ;
    }
    
    if ( use_open_gl )
    {
#ifdef HAVE_LIBGL
	/* The old approach was to malloc a buffer the size of the screen and glReadPixels to it. The problem
	is that it's awfully slow, and subsequent RestoreMenuBackground will be slow as well. Here, we hope the
	graphics card has enough RAM, and just create a big texture to store the image.
	*/

        glFlush();

	if ( StoredMenuBackgroundTex [ backup_slot ] == 0 )
		{ 
		StoredMenuBackgroundTex [ backup_slot ] = &all_freedroid_textures [ next_texture_index_to_use ] ;
	        next_texture_index_to_use ++ ;
		if ( next_texture_index_to_use >= MAX_AMOUNT_OF_TEXTURES_WE_WILL_USE )
		    {
                    ErrorMessage ( __FUNCTION__  ,
                                   "Ran out of initialized texture positions to use for new textures.",
                                   PLEASE_INFORM, IS_FATAL );
                    } 
		}

	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, *StoredMenuBackgroundTex [ backup_slot ]);
        glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_NEAREST );

	// We assume the GL setup supports 2048x2048 textures. If it's not the case, heh... change your graphics card :)
	int txw = (GameConfig . screen_width > 1024) ? 2048 : 1024;
	int txh = (GameConfig . screen_height > 1024 ) ? 2048 : 1024;

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, GameConfig.screen_height - txh, txw,  txh, 0);
	open_gl_check_error_status(__FUNCTION__);

#endif
    }
    else
    {
	//--------------------
	// If the memory was not yet allocated, we need to do that now...
	//
	// otherwise we free the old surface and create a new copy of the
	// current screen content...
	//
	if ( StoredMenuBackground [ backup_slot ] == NULL )
	{
	    StoredMenuBackground [ backup_slot ] = SDL_DisplayFormat ( Screen );
	}
	else
	{
	    SDL_FreeSurface ( StoredMenuBackground [ backup_slot ] );
	    StoredMenuBackground [ backup_slot ] = SDL_DisplayFormat ( Screen );
	}
    }
    
}; // void StoreMenuBackground ( int backup_slot )

/**
 * Following a suggestion from Simon, we're now implementing one single
 * small texture (to be modified with pixel operations every frame) that
 * can be stretched out over the whole screen via OpenGL.
 * This function is here to set up the texture in the first place.
 */
void
set_up_stretched_texture_for_light_radius ( void )
{
#ifdef HAVE_LIBGL

    static int texture_is_set_up_already = FALSE ;

    //--------------------
    // In the non-open-gl case, this function shouldn't be called ever....
    //
    if ( ! use_open_gl ) return;

    //--------------------
    // Some protection against creating this texture twice...
    //
    if ( texture_is_set_up_already ) return ;
    texture_is_set_up_already = TRUE ;

    //--------------------
    // We create an SDL surface, so that we can make the texture for the
    // stretched-texture method light radius from it...
    //
    light_radius_stretch_surface = SDL_CreateRGBSurface( SDL_SWSURFACE , LIGHT_RADIUS_STRETCH_TEXTURE_WIDTH , LIGHT_RADIUS_STRETCH_TEXTURE_HEIGHT , 32, rmask , gmask, bmask , amask );

    //--------------------
    // Having prepared the raw image it's now time to create the real
    // textures.
    //
    glPixelStorei( GL_UNPACK_ALIGNMENT , 1 );

    light_radius_stretch_texture = & ( all_freedroid_textures [ next_texture_index_to_use ] ) ;
    next_texture_index_to_use ++ ;

    if ( next_texture_index_to_use >= MAX_AMOUNT_OF_TEXTURES_WE_WILL_USE )
    {
	ErrorMessage ( __FUNCTION__  , 
				   "Ran out of initialized texture positions to use for new textures.",
				   PLEASE_INFORM, IS_FATAL );
    }
    else
    {
	DebugPrintf ( 0 , "\nTexture positions remaining: %d." , MAX_AMOUNT_OF_TEXTURES_WE_WILL_USE - next_texture_index_to_use );
    }
    
    glBindTexture( GL_TEXTURE_2D, * ( light_radius_stretch_texture ) );
  
    glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );

    // Generate The Texture 
    glTexImage2D( GL_TEXTURE_2D, 0, 4, light_radius_stretch_surface -> w,
		  light_radius_stretch_surface -> h, 0, GL_BGRA,
		  GL_UNSIGNED_BYTE, light_radius_stretch_surface -> pixels );


#endif


}; // void set_up_stretched_texture_for_light_radius ( void )

/**
 * This function updated the automap texture, such that all info from the
 * current square is on the automap.
 */
void
light_radius_update_stretched_texture ( void ) 
{
#ifdef HAVE_LIBGL
    int x , y ;
    int red = 0 ; 
    int blue = 0 ;
    int green = 0 ;
    int alpha = 0 ;
    moderately_finepoint target_pos ;
    int light_strength ;
    int lr_square_width = GameConfig . screen_width / 64;
    int lr_square_height = GameConfig . screen_height / 48;
    static float alpha_factor = 255.0 / (float) NUMBER_OF_SHADOW_IMAGES;
    //--------------------
    // Now it's time to edit the automap texture.
    //
    for ( x = 0 ; x < 64 ; x ++ )
    {
	for ( y = 0 ; y < 48 ; y ++ )
	{
	    target_pos . x = translate_pixel_to_map_location ( ( 1 + x ) * lr_square_width - UserCenter_x - User_Rect.x / 2 - (User_Rect . x + User_Rect.w - GameConfig . screen_width) / 2,
							       ( 0 + y ) * lr_square_height - UserCenter_y, TRUE );
	    target_pos . y = translate_pixel_to_map_location ( ( 1 + x ) * lr_square_width - UserCenter_x - User_Rect . x / 2 - (User_Rect . x + User_Rect.w - GameConfig . screen_width) / 2,
							       ( 0 + y ) * lr_square_height - UserCenter_y, FALSE );

	    light_strength = get_light_strength ( target_pos );
	    
	    if ( light_strength >= NUMBER_OF_SHADOW_IMAGES ) light_strength = NUMBER_OF_SHADOW_IMAGES -1 ;
	    if ( light_strength <= 0 ) light_strength = 0 ;
	    
            alpha = ( alpha_factor ) * ( (float) light_strength ) ; 

	    PutPixel32 ( light_radius_stretch_surface , x , LIGHT_RADIUS_STRETCH_TEXTURE_HEIGHT - y - 1 , 
		       SDL_MapRGBA ( light_radius_stretch_surface -> format , red , green , blue , alpha ) ) ;

	}
    }

    glBindTexture ( GL_TEXTURE_2D , *light_radius_stretch_texture );
    glTexSubImage2D ( GL_TEXTURE_2D , 0 , 
		      0  , 0 ,
		      64 ,
		      64 ,
		      GL_RGBA, 
		      GL_UNSIGNED_BYTE, 
		      light_radius_stretch_surface -> pixels );

    open_gl_check_error_status ( __FUNCTION__ );

#endif

}; // void light_radius_update_stretched_texture ( void ) 

/**
 * Following a suggestion from Simon, we're now implementing one single
 * small texture (to be modified with pixel operations every frame) that
 * can be stretched out over the whole screen via OpenGL.
 * This function is here to set up the texture in the first place.
 */
void
blit_open_gl_stretched_texture_light_radius ( void )
{
#ifdef HAVE_LIBGL
    iso_image local_iso_image;
    
    //--------------------
    // We make sure, that there is one single texture created before
    // doing any of our texture-blitting or texture-modification stuff
    // with it.
    //
    set_up_stretched_texture_for_light_radius ( );

    light_radius_update_stretched_texture ( ) ;

    //--------------------
    // Now we blit the current automap texture to the screen.  We use standard
    // texture blitting code for this, so we need to embed the automap texture
    // in a surrounting 'iso_image', but that shouldn't be costly or anything...
    //
    local_iso_image . texture = light_radius_stretch_texture ;
    local_iso_image . texture_width = LIGHT_RADIUS_STRETCH_TEXTURE_WIDTH ;
    local_iso_image . texture_height = LIGHT_RADIUS_STRETCH_TEXTURE_HEIGHT ;
    local_iso_image . original_image_width = LIGHT_RADIUS_STRETCH_TEXTURE_WIDTH ;
    local_iso_image . original_image_height = LIGHT_RADIUS_STRETCH_TEXTURE_HEIGHT ;
    local_iso_image . texture_has_been_created = TRUE ;
    local_iso_image . offset_x = 0 ;    
    local_iso_image . offset_y = 0 ;

    glEnable ( GL_BLEND ) ;

    draw_gl_scaled_textured_quad_at_screen_position ( 
	& local_iso_image , 
	0,
	-60 * (GameConfig . screen_height / 768),
	((float)GameConfig . screen_width) / ((float)LIGHT_RADIUS_STRETCH_TEXTURE_WIDTH) );
    glDisable ( GL_BLEND ) ;

#endif

}; // void blit_open_gl_stretched_texture_light_radius ( void )

/**
 * This function should help to do 'putpixel' even with OpenGL graphics
 * output method.  However, using that is depreciated if it can be helped
 * somehow.
 */
void
PutPixel_open_gl ( int x, int y, Uint32 pixel)
{

#ifdef HAVE_LIBGL
    /* Uglier and slower than glDrawPixels, but it works :) */
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_POINTS);
    glColor3ub(((pixel >> 16) & 0xff), (pixel >> 8) & 0xff, (pixel) & 0xff);
    glVertex2i(x, y);
    glEnd();
    glEnable(GL_TEXTURE_2D); 
#endif
    
}; // void PutPixel_open_gl ( x , y , pixel ) ;


/**
 * On various occasions, like inside the shop interface or in the level
 * editor tile selection bar, it's nescessary to mark an item as currently
 * selected.  We do this by drawing a part-transparent white rectangle 
 * right over it.  This function can be used to draw that rectangle.
 */
void
GL_HighlightRectangle ( SDL_Surface* Surface , SDL_Rect * dstrect , unsigned char r , unsigned char g , unsigned char b , unsigned char alpha )
{
#ifdef HAVE_LIBGL

    glDisable ( GL_TEXTURE_2D );
    glEnable ( GL_BLEND );
    
    glColor4ub( r , g , b , alpha );

    glBegin(GL_QUADS);
    glVertex2i( dstrect -> x                , dstrect -> y );
    glVertex2i( dstrect -> x                , dstrect -> y + dstrect -> h );
    glVertex2i( dstrect -> x + dstrect -> w , dstrect -> y + dstrect -> h );
    glVertex2i( dstrect -> x + dstrect -> w , dstrect -> y );
    glEnd( );

    glEnable( GL_TEXTURE_2D );
    glDisable ( GL_BLEND );

#endif

    return;

}; // void GL_HighlightRectangle


#define CHARACTER_SCREEN_BACKGROUND_FILE "backgrounds/character.png" 
#define SKILL_SCREEN_BACKGROUND_FILE "backgrounds/SkillScreen.png" 
#define SKILL_EXPLANATION_SCREEN_BACKGROUND_FILE "backgrounds/SkillExplanationScreen.png" 
#define INVENTORY_SCREEN_BACKGROUND_FILE "backgrounds/inventory.png"
#define NE_TITLE_PIC_FILE       "backgrounds/title.jpg"
#define NE_CREDITS_PIC_FILE     "backgrounds/credits.jpg"
#define SHOP_BACKGROUND_IMAGE   "backgrounds/shoppe.jpg"
#define ITEM_BROWSER_BG_PIC_FILE "backgrounds/item_browser.jpg"
#define ITEM_BROWSER_SHOP_FILE "backgrounds/item_browser_shop.png"
#define NE_CONSOLE_FG_1_FILE     "backgrounds/console_fg_1.png" 
#define NE_CONSOLE_FG_2_FILE     "backgrounds/console_fg_2.png" 
#define NE_CONSOLE_FG_3_FILE     "backgrounds/console_fg_3.png" 
#define NE_CONSOLE_FG_4_FILE     "backgrounds/console_fg_4.png" 
#define NE_CONSOLE_BG_PIC1_FILE "backgrounds/console_bg1.jpg"
#define NE_CONSOLE_BG_PIC2_FILE "backgrounds/console_bg2.jpg"
#define LEVEL_EDITOR_BANNER_FILE1 "backgrounds/LevelEditorSelectionBar1.png"
#define LEVEL_EDITOR_BANNER_FILE2 "backgrounds/LevelEditorSelectionBar2.png"
#define LEVEL_EDITOR_BANNER_FILE3 "backgrounds/LevelEditorSelectionBar3.png"
#define LEVEL_EDITOR_BANNER_FILE4 "backgrounds/LevelEditorSelectionBar4.png"
#define LEVEL_EDITOR_BANNER_FILE5 "backgrounds/LevelEditorSelectionBar5.png"
#define LEVEL_EDITOR_BANNER_FILE6 "backgrounds/LevelEditorSelectionBar6.png"
#define LEVEL_EDITOR_BANNER_FILE7 "backgrounds/LevelEditorSelectionBar7.png"
#define FREEDROID_LOADING_PICTURE_NAME "backgrounds/startup1.jpg"
#define MOUSE_BUTTON_CHA_BACKGROUND_PICTURE "mouse_buttons/CHAButton.png"           
#define MOUSE_BUTTON_INV_BACKGROUND_PICTURE "mouse_buttons/INVButton.png"           
#define MOUSE_BUTTON_SKI_BACKGROUND_PICTURE "mouse_buttons/SKIButton.png"           
#define MOUSE_BUTTON_PLUS_BACKGROUND_PICTURE "mouse_buttons/PLUSButton.png"          
#define CHAT_BACKGROUND_IMAGE_FILE "backgrounds/conversation.png"
#define TO_BG_FILE		"to_background.jpg"
#define QUEST_BROWSER_BACKGROUND_IMAGE_FILE "backgrounds/quest_browser.png"
#define NUMBER_SELECTOR_BACKGROUND_IMAGE_FILE "backgrounds/number_selector.png"
#define GAME_MESSAGE_WINDOW_BACKGROUND_IMAGE_FILE "backgrounds/game_message_window.png"
#define HUD_BACKGROUND_IMAGE_FILE "backgrounds/hud_background.png"

#define ALL_KNOWN_BACKGROUNDS 33
static iso_image our_backgrounds [ ALL_KNOWN_BACKGROUNDS ] ;
static int background_has_been_loaded [ ALL_KNOWN_BACKGROUNDS ] ;

/**
 * For blitting backgrounds and static images in various positions of the
 * game, we got this function, that handles them, taking especal care to
 * use open-gl textures for faster blitting in OpenGL settings.
 */
void 
blit_special_background ( int background_code )
{
    SDL_Surface* tmp_surf_1;
    SDL_Rect src_rect;
char fpath[2048];

    if ( background_code >= ALL_KNOWN_BACKGROUNDS ) 
	{
	ErrorMessage ( __FUNCTION__  ,
                                   "Received a request to display a background that does not exist.",
                                   PLEASE_INFORM, IS_FATAL );
	exit(1);
	}
	
    static char* background_filenames [ ALL_KNOWN_BACKGROUNDS ] = 
	{ 
	    INVENTORY_SCREEN_BACKGROUND_FILE ,  // 0
	    CHARACTER_SCREEN_BACKGROUND_FILE ,  // 1 
	    SKILL_SCREEN_BACKGROUND_FILE ,      // 2
	    SKILL_EXPLANATION_SCREEN_BACKGROUND_FILE , // 3
	    NE_TITLE_PIC_FILE ,                 // 4
	    NE_CREDITS_PIC_FILE ,               // 5
	    SHOP_BACKGROUND_IMAGE ,             // 6
	    ITEM_BROWSER_BG_PIC_FILE ,          // 7
	    ITEM_BROWSER_SHOP_FILE ,            // 8 
	    NE_CONSOLE_FG_1_FILE ,              // 9 
	    NE_CONSOLE_FG_2_FILE ,              // 10
	    NE_CONSOLE_FG_3_FILE ,              // 11
	    NE_CONSOLE_FG_4_FILE ,              // 12
	    NE_CONSOLE_BG_PIC1_FILE ,           // 13
	    LEVEL_EDITOR_BANNER_FILE1 ,          // 14
	    LEVEL_EDITOR_BANNER_FILE2 ,          // 15
	    LEVEL_EDITOR_BANNER_FILE3 ,          // 16
	    LEVEL_EDITOR_BANNER_FILE4 ,          // 17
	    LEVEL_EDITOR_BANNER_FILE5 ,          // 18
	    LEVEL_EDITOR_BANNER_FILE6 ,          // 19
	    LEVEL_EDITOR_BANNER_FILE7 ,          // 20
	    FREEDROID_LOADING_PICTURE_NAME ,    // 21
	    MOUSE_BUTTON_CHA_BACKGROUND_PICTURE , // 22
	    MOUSE_BUTTON_INV_BACKGROUND_PICTURE , // 23
	    MOUSE_BUTTON_SKI_BACKGROUND_PICTURE , // 24 
	    MOUSE_BUTTON_PLUS_BACKGROUND_PICTURE , // 25
	    CHAT_BACKGROUND_IMAGE_FILE ,        // 26
	    CHAT_BACKGROUND_IMAGE_FILE ,        // 27
	    TO_BG_FILE ,                        // 28
	    QUEST_BROWSER_BACKGROUND_IMAGE_FILE, // 29
	    NUMBER_SELECTOR_BACKGROUND_IMAGE_FILE, // 30
	    GAME_MESSAGE_WINDOW_BACKGROUND_IMAGE_FILE, // 31
	    HUD_BACKGROUND_IMAGE_FILE, // 32
	};

    SDL_Rect our_background_rects [ ALL_KNOWN_BACKGROUNDS ] = 
	{ 
	    { 0 , 0 , 0 , 0 } ,               // 0
	    { CHARACTERRECT_X , 0 , 0 , 0 } , // 1 
	    { CHARACTERRECT_X , 0 , 0 , 0 } , // 2
	    { 0 , 0 , 0 , 0 } ,               // 3
	    { 0 , 0 , 0 , 0 } ,               // 4 
	    { 0 , 0 , 0 , 0 } ,               // 5
	    { 0 , 0 , 0 , 0 } ,               // 6
	    { 0 , 0 , 0 , 0 } ,               // 7
	    { 0 , 0 , 0 , 0 } ,               // 8
	    
	    { 32, 180, CONS_MENU_LENGTH, CONS_MENU_HEIGHT } , // 9
	    { 32, 180, CONS_MENU_LENGTH, CONS_MENU_HEIGHT } , // 10
	    { 32, 180, CONS_MENU_LENGTH, CONS_MENU_HEIGHT } , // 11
	    { 32, 180, CONS_MENU_LENGTH, CONS_MENU_HEIGHT } , // 12
	    { 0 , 0 , 0 , 0 } ,               // 13
	    { 0 , 0 , 0 , 0 } ,               // 14
	    { 0 , 0 , 0 , 0 } ,               // 15
	    { 0 , 0 , 0 , 0 } ,               // 16
	    { 0 , 0 , 0 , 0 } ,               // 17
	    { 0 , 0 , 0 , 0 } ,               // 18
	    { 0 , 0 , 0 , 0 } ,               // 19
	    { 0 , 0 , 0 , 0 } ,               // 20
	    { 0 , 0 , 0 , 0 } ,               // 21
	    { GameConfig . screen_width - 80 , GameConfig . screen_height - 46 ,  38 ,  45 } , // 22
	    { GameConfig . screen_width - 40 , GameConfig . screen_height - 60 ,  38 ,  40 } , // 23 
	    { GameConfig . screen_width - 50 , GameConfig . screen_height - 104 ,  38 ,  47 } , // 24
	    { GameConfig . screen_width - 80 , GameConfig . screen_height - 46 ,  38 ,  45 } , // 25
	    { 0 , 0 , 0 , 0 } ,               // 26
	    { CHAT_SUBDIALOG_WINDOW_X , 
	      CHAT_SUBDIALOG_WINDOW_Y , 
	      CHAT_SUBDIALOG_WINDOW_W , 
	      CHAT_SUBDIALOG_WINDOW_H } ,     // 27
	    { 0 , 0 , 0 , 0 } ,               // 28
	    { 0 , 0 , 0 , 0 } ,               // 29
	    { 0 , 0 , 0 , 0 } ,               // 30
	    { ( 65 * GameConfig . screen_width ) / 640 , GameConfig . screen_height - (70 * GameConfig . screen_height) / 480 , 500 , 70 } ,          // 31
	    { ( 0 * GameConfig . screen_width ) / 640 , GameConfig . screen_height - ( 118 * GameConfig . screen_height) / 480 , 500 , 70 }           // 32
	} ;
  
    const int need_scaling [ ALL_KNOWN_BACKGROUNDS ] = 
	{
	    FALSE , // 0
	    FALSE , // 1
	    FALSE , // 2
	    FALSE , // 3
	    TRUE  , // 4
	    TRUE  , // 5
	    TRUE  , // 6
	    TRUE  , // 7
	    TRUE  , // 8
	    FALSE , // 9
	    FALSE , // 10
	    FALSE , // 11
	    FALSE , // 12
	    FALSE , // 13
	    FALSE , // 14
	    FALSE , // 15
	    FALSE , // 16
	    FALSE , // 17
	    FALSE , // 18
	    FALSE , // 19
	    FALSE , // 20
	    TRUE  , // 21
	    FALSE , // 22
	    FALSE , // 23
	    FALSE , // 24
	    FALSE , // 25
	    TRUE  , // 26
	    TRUE  , // 27
	    TRUE  , // 28
	    TRUE  , // 29
	    TRUE , // 30
	    TRUE  , // 31
	    TRUE  , // 32
	};
	    
    if ( ! background_has_been_loaded [ background_code] )
    {
	background_has_been_loaded [ background_code ] = 1; 
	    
	    find_file (background_filenames [ background_code ] , GRAPHICS_DIR , fpath, 0 );
	    get_iso_image_from_file_and_path ( fpath , & ( our_backgrounds [ background_code ] ) , FALSE ) ;
	    
	    //--------------------
	    // For the dialog, we need not only the dialog background, but also some smaller
	    // parts of the background image, so we can re-do the background part that is in
	    // the dialog partners chat output window.  We don't make a separate image on disk
	    // but rather extract the info inside the code.  That makes for easier adaption
	    // of the window dimensions from inside the code...
	    //
	    if ( background_code == CHAT_DIALOG_BACKGROUND_EXCERPT_CODE )
	    {
		tmp_surf_1 = SDL_CreateRGBSurface ( SDL_SWSURFACE , CHAT_SUBDIALOG_WINDOW_W , CHAT_SUBDIALOG_WINDOW_H , 
						    32 , 0x000000ff , 0x0000ff00 , 0x00ff0000 , 0xff000000 );
		
		src_rect . x = CHAT_SUBDIALOG_WINDOW_X ;
		src_rect . w = CHAT_SUBDIALOG_WINDOW_W ;
		src_rect . h = CHAT_SUBDIALOG_WINDOW_H ;
		//--------------------
		// With OpenGL, the image is flipped at this point already, so we
		// just copy the image in flipped form, cause later it should be 
		// flipped anyway.  Cool, eh?  Of course this way only the location
		// of the rectangle has to be adapted a bit...
		//
		if ( use_open_gl )
		{
		    src_rect . y = 480 - CHAT_SUBDIALOG_WINDOW_Y - CHAT_SUBDIALOG_WINDOW_H ;
		}
		else
		{
		    src_rect . y = CHAT_SUBDIALOG_WINDOW_Y ;
		}
		
		SDL_BlitSurface ( our_backgrounds [ background_code ] . surface , &src_rect , tmp_surf_1 , NULL );
		SDL_FreeSurface ( our_backgrounds [ background_code ] . surface ) ;
		our_backgrounds [ background_code ] . surface = SDL_DisplayFormat ( tmp_surf_1 );
	    }
	    
	    if ( use_open_gl )
	    {
		make_texture_out_of_surface ( & ( our_backgrounds [ background_code ] ) ) ;
	    }
	
    }
    
    if ( use_open_gl )
    {
	if ( need_scaling [ background_code ] )
	{
	    draw_gl_bg_textured_quad_at_screen_position ( 
		&our_backgrounds [ background_code ] , 
		our_background_rects [ background_code ] . x , 
		our_background_rects [ background_code ] . y ) ;
	}
	else
	    draw_gl_textured_quad_at_screen_position ( 
		&our_backgrounds [ background_code ] , 
		our_background_rects [ background_code ] . x , 
		our_background_rects [ background_code ] . y ) ;
    }
    else
    {
	if ( need_scaling [ background_code ] &&  our_background_rects [ background_code ] . w == 0 && our_backgrounds[background_code] . original_image_width == 1024) //we are using one of the new fullscreen backgrounds
        	{
		SDL_Surface * tmp_surf2 = zoomSurface ( our_backgrounds [ background_code ] . surface , 0.625, 0.625 , FALSE );
		our_background_rects [ background_code ] . w = 640;
		our_background_rects [ background_code ] . h = 480;
		SDL_FreeSurface ( our_backgrounds [ background_code ] . surface );
		our_backgrounds [ background_code ] . surface = tmp_surf2;
		our_backgrounds[background_code] . original_image_width = 640;
		our_backgrounds[background_code] . original_image_height = 480;
		}
	SDL_SetClipRect( Screen, NULL );
	our_SDL_blit_surface_wrapper ( our_backgrounds [ background_code] . surface , NULL , Screen , &( our_background_rects [ background_code ] ) );

    }
  
}; // void blit_special_background ( int background_code )

/**
 * Sometimes it might be convenient for development purposes, that the
 * backgrounds can be exchanged on disk and the game need not be restarted
 * to try out these new backgrounds.  So we introduce some 'cache flushing'
 * function here...
 */
void
flush_background_image_cache ( void )
{
    int i;
    static iso_image empty_image = UNLOADED_ISO_IMAGE ;
    
    //--------------------
    // Also we dutifully set all background variables to 'empty'
    // status, cause re-initializing stuff might cause error or
    // warning messages, if the 'has_been_created' flags are still
    // set from last time...
    //
    for ( i = 0 ; i < ALL_KNOWN_BACKGROUNDS ; i ++ )
    {
	background_has_been_loaded [ i ] = 0;
	memcpy ( & ( our_backgrounds [ i ] ) , & empty_image , sizeof ( iso_image ) );
    }
    
}; // void flush_background_image_cache ( void )

#undef _open_gl_c
