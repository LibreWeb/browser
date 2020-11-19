#include "md-render.h"
#include "scene.h"
#include "cmark-gfm.h"

#include <node.h>
#include <QFont>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QPainter>

Renderer::Renderer(Scene* scene) : 
    scene(scene),
    sceneMarginX(3.0),
    sceneMarginY(3.0),
    bold(false),
    italic(false),
    headingLevel(0),
    listLevel(0),
    currentX(0.0),
    currentY(0.0),
    fontFamilty("Open Sans"),
    wordSpacing(4.0), // spacing may depend on the font
    heighestHigh(0.0),
    paragraphHeightOffset(5.0),
    headingHeightOffset(10.0),
    listXOffset(15.0),
    bulletWithTemp(0.0) {
}

/**
 * Render the whole document to scene/screen
 */
void Renderer::renderDocument(cmark_node *root, int width)
{
    cmark_event_type ev_type;
    cmark_iter *iter = cmark_iter_new(root);
    while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE) {
        cmark_node *cur = cmark_iter_get_node(iter);
        renderNode(cur, ev_type);
    }
}

/**
 * Calculates the locations, render and paint the content/objects
 * to a QGraphicsScene
 */
void Renderer::renderNode(cmark_node *node, cmark_event_type ev_type)
{
    bool entering = (ev_type == CMARK_EVENT_ENTER);
    
    switch (node->type) {
    case CMARK_NODE_DOCUMENT:
        if (entering) {
            currentX = sceneMarginX;
            currentY = sceneMarginY;
        }
        break;

    case CMARK_NODE_BLOCK_QUOTE:
        break;

    case CMARK_NODE_LIST:
        if (entering) {
            listLevel++;
        } else {
            listLevel--;
            if (listLevel < 0)
                listLevel = 0;
        }

        if (listLevel == 0) {
            // Reset X to be safe
            currentX = sceneMarginX;
        } else if (listLevel > 0) {
            if (entering) {
                currentX += listXOffset;
            } else {
                currentX -= listXOffset;
            }
        }
        break;

    case CMARK_NODE_ITEM:
        // Line break for list items
        currentY += heighestHigh;
        // Reset heighest high (Y-axis)
        heighestHigh = 0;

        // Add bullet before text items
        if (entering) {
            const QRectF rec = drawBullet();
            bulletWithTemp = rec.width() + 2.0; // + offset
            currentX += bulletWithTemp;
        } else {
            currentX -= bulletWithTemp;
        }
        break;

    case CMARK_NODE_HEADING:
        if (entering) {
            headingLevel = node->as.heading.level;
        } else {
            headingLevel = 0; // reset
        }
        // Move to left again
        currentX = sceneMarginX;
        // New heading
        currentY += heighestHigh + headingHeightOffset;
        
        // Reset heighest high (Y-axis)
        heighestHigh = 0;

        break;

    case CMARK_NODE_CODE_BLOCK:
        break;

    case CMARK_NODE_HTML_BLOCK:
        break;

    case CMARK_NODE_CUSTOM_BLOCK:
        break;

    case CMARK_NODE_THEMATIC_BREAK:
        break;

    case CMARK_NODE_PARAGRAPH:
        printf("p\n");

        // Skip paragraph if listing is enabled
        if (listLevel == 0) {
            // Move to left again
            currentX = sceneMarginX;
            // New paragraph
            currentY += heighestHigh + paragraphHeightOffset;
            
            // Reset heighest high (Y-axis)
            heighestHigh = 0;
        }
        break;

    case CMARK_NODE_TEXT: {
            printf("text\n");
            const QRectF rec = drawText(cmark_node_get_literal(node));
            // Skip paragraph if listing is enabled
            if (listLevel == 0) {
                currentX += rec.width();
            }
            if (rec.height() > heighestHigh)
                heighestHigh = rec.height();
        }
        break;

    case CMARK_NODE_LINEBREAK:
        // Move to left again
        currentX = sceneMarginX;
        // Line break (no soft break)
        currentY += heighestHigh;
        
        // Reset heighest high (Y-axis)
        heighestHigh = 0;
        break;

    case CMARK_NODE_SOFTBREAK:
        // ignore
        // Only insert a space between the words
        currentX += wordSpacing;
        break;

    case CMARK_NODE_CODE:
        break;

    case CMARK_NODE_HTML_INLINE:
        break;

    case CMARK_NODE_CUSTOM_INLINE:
        break;

    case CMARK_NODE_STRONG:
        bold = entering;
        break;

    case CMARK_NODE_EMPH:
        italic = entering;
        break;

    case CMARK_NODE_LINK:
        break;

    case CMARK_NODE_IMAGE:
        break;

    case CMARK_NODE_FOOTNOTE_REFERENCE:
        break;

    case CMARK_NODE_FOOTNOTE_DEFINITION:
        break;
    default:
        assert(false);
        break;
    }
}

QRectF const Renderer::drawText(const QString& text)
{
    // We can still extend the QGraphicsSimpleTextItem class (or QAbstractGraphicsShapeItem) and override paint method.
    // Or just use QPainter with a paint device (like QWidgets), to have maximal control.
    QGraphicsSimpleTextItem *textItem = new QGraphicsSimpleTextItem(text);
    QFont font;
    if (bold)
        font.setBold(true);
    if (italic)
        font.setItalic(true);
    
    if (headingLevel > 0) {
        font.setBold(true);
        switch(headingLevel) {
        case 1:
            font.setPixelSize(24);
            break;
        case 2:
            font.setPixelSize(20);
            break;
        case 3:
            font.setPixelSize(16);
            break;
        case 4:
            font.setPixelSize(14);
            break;
        case 5:
            font.setPixelSize(12);
            break;
        default:
            break;
        }
    }

    font.setFamily(fontFamilty);
    textItem->setFont(font);
    textItem->setPos(currentX, currentY);
    scene->addItem(textItem);

    return textItem->boundingRect();
}

QRectF const Renderer::drawBullet()
{
    QGraphicsSimpleTextItem *bullet = new QGraphicsSimpleTextItem("\u2022");
    QFont font;
    font.setFamily(fontFamilty);
    bullet->setFont(font);
    bullet->setPos(currentX, currentY);
    scene->addItem(bullet);
    return bullet->boundingRect();
}