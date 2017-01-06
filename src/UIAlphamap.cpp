#include "UIAlphamap.h"

#include <cstring>

#include "Video.h"
#include "World.h"
#include "MapIndex.h"
#include "MapTile.h"
#include "MapChunk.h"
#include "TextureSet.h"

UIAlphamap::UIAlphamap(float xPos, float yPos)
	: UICloseWindow(xPos, yPos, 600, 600, "Alphamap", true)
{

}

void UIAlphamap::render() const
{
	UICloseWindow::render();

  tile_index tile(gWorld->camera);
	float colorf[3];

	if (!gWorld->mapIndex->tileLoaded(tile))
		return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	for (size_t j = 0; j < 16; ++j)
	{
		for (size_t i = 0; i < 16; ++i)
		{
      TextureSet* tex = gWorld->mapIndex->getTile(tile)->getChunk(i, j)->textureSet;
			for (size_t t = 0; t < tex->num() - 1; ++t)
			{
				memset(colorf, 0, 3 * sizeof(float));
				colorf[t] = 1.0f;

				glColor3fv(colorf);

				tex->bindAlphamap(t, 0);
				drawQuad(i, j);
			}
		}
	}

	OpenGL::Texture::disableTexture(0);
}

void UIAlphamap::drawQuad(size_t i, size_t j) const
{
	int size = 512;
	int unit = size / 16;
	int border = (int)((width() - size) / 2);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(x() + border + i*unit, y() + border + j*unit);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(x() + border + i*unit + unit, y() + border + j*unit);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(x() + border + i*unit + unit, y() + border + j*unit + unit);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(x() + border + i*unit, y() + border + j*unit + unit);
	glEnd();
}
