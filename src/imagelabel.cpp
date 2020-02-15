#include "imagelabel.h"

ImageLabel::ImageLabel(QWidget *parent) :
    QLabel(parent)
{
    setMinimumSize(1,1);
    setAlignment(Qt::AlignCenter);
    setMouseTracking(true);
    // This is important to preserve the aspect ratio
    setScaledContents(true);

    // Dark background
    setBackgroundRole(QPalette::Shadow);

    // Size policy
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    setFocusPolicy(Qt::StrongFocus);

    setCursor(Qt::CrossCursor);
}

void ImageLabel::setPixmap ( QPixmap & p)
{
    base_pixmap = p;

    drawLabel();
    resize(pixmap()->size());
}

int ImageLabel::heightForWidth( int width ) const
{
    return base_pixmap.isNull() ? height() : static_cast<int>((static_cast<qreal>(base_pixmap.height())*width)/base_pixmap.width());
}

QSize ImageLabel::sizeHint() const
{
    int w = width();
    return QSize( w, heightForWidth(w) );
}

void ImageLabel::resizeEvent(QResizeEvent * e)
{
    if(!base_pixmap.isNull()){
        drawLabel();
    }else{
        e->ignore();
    }
}

void ImageLabel::zoom(double factor){
    zoom_factor = factor;
    scaledPixmap();

    if(zoom_factor == 1.0){
        resize(base_pixmap.size());
    }else{
        resize(scaled_pixmap.size());
    }

}

QPixmap ImageLabel::scaledPixmap(void)
{

    if(base_pixmap.isNull())
        return QPixmap();

    if(shouldScaleContents){
        scaled_pixmap = base_pixmap.scaled( size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }else if(zoom_factor != 1.0){
        scaled_pixmap = base_pixmap.scaled( zoom_factor*base_pixmap.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }else{
        scaled_pixmap = base_pixmap;
    }

    scaled_width = scaled_pixmap.width();
    scaled_height = scaled_pixmap.height();

    scale_factor = static_cast<float>(scaled_height)/base_pixmap.height();

    return scaled_pixmap;
}

QPoint ImageLabel::getScaledImageLocation(QPoint location){
    // If the image is fit to window

    QPoint scaled_location = location;

    if(scale_factor != 1.0){

        // Get the location on the image, accounting for padding
        scaled_location.setX(scaled_location.x() - (width() - scaled_width)/2);
        scaled_location.setY(scaled_location.y() - (height() - scaled_height)/2);

        scaled_location.setX(static_cast<int>(scaled_location.x() / scale_factor));
        scaled_location.setY(static_cast<int>(scaled_location.y() / scale_factor));
    }

    return scaled_location;
}

bool ImageLabel::searchBboxes(QPoint location, QList<BoundingBox> bboxes, int& position) {
    // TODO (Kamal): Optimize search
    for (int i = 0; i<bboxes.size(); i++) {
        if(bboxes[i].rect.contains(location)) {
            position = i;
            return true;
        }
    }
    return false;
}

bool ImageLabel::searchBboxes(BoundingBox bbox, QList<BoundingBox> bboxes, int& position) {
    // TODO (Kamal): Optimize search
    for (int i = 0; i<bboxes.size(); i++) {
        if(bboxes[i].rect == bbox.rect) {
            position = i;
            return true;
        }
    }
    return false;
}

interactionState ImageLabel::checkMode(QPoint image_location)
{
    // Select Mode: location within bounding boxes
    // Draw Mode: location outside bounding boxes
    scaledPixmap();

    if(scale_factor == 1.0){
        scaled_pixmap = base_pixmap;
    }
    same_box = false;
    bool found = false;

    BoundingBox bbox;

    if(selected){
        // Slightly bigger box to take anchors
        // into consideration
        QList<BoundingBox> biggerBboxes;
        for (auto bbox: bboxes){
            BoundingBox bigBbox;
            bigBbox.rect = bbox.rect + QMargins(10, 10, 10, 10);
            biggerBboxes.append(bigBbox);
        }
        found = searchBboxes(image_location, biggerBboxes, editbbox_position);
    }else{
        // Search within original boxes
        found = searchBboxes(image_location, bboxes, editbbox_position);
    }

    if(found){
        editbbox = &bboxes[editbbox_position];
        original_box = bboxes[editbbox_position];
        if(selected_bbox.rect == original_box.rect)
            same_box = true;
        else
            same_box = false;
        return interactionState::MODE_SELECT;
    }

    return interactionState::MODE_DRAW;
}

void ImageLabel::drawEditLabel(BoundingBox bbox){

    // display selected bounding box
    scaledPixmap();
    drawBoundingBox(bbox, Qt::green, MODE_SELECT);
    QLabel::setPixmap(scaled_pixmap);
}

void ImageLabel::drawBoundingBox(BoundingBox bbox, QColor colour, interactionState mode){

    // draw bounding box with specific color
    if(scaled_pixmap.isNull()) return;

    QPainter painter;
    painter.begin(&scaled_pixmap);
    QPen pen(colour, 2);
    painter.setPen(pen);

    auto scaled_bbox = bbox.rect;

    scaled_bbox.setRight(static_cast<int>(scaled_bbox.right() * scale_factor));
    scaled_bbox.setLeft(static_cast<int>(scaled_bbox.left() * scale_factor));
    scaled_bbox.setTop(static_cast<int>(scaled_bbox.top() * scale_factor));
    scaled_bbox.setBottom(static_cast<int>(scaled_bbox.bottom() * scale_factor));

    if(bbox.classname != ""){

        painter.setFont(QFont("Helvetica", 12));
        painter.drawText(scaled_bbox.bottomLeft(), bbox.classname);
        painter.drawText(scaled_bbox.topRight(), QString::number(bbox.id));

    }

    painter.drawRect(scaled_bbox);

    // Draw anchors upon selection
    if(mode == MODE_SELECT){
        pen.setWidth(7);
        painter.setPen(pen);
        painter.drawPoint(scaled_bbox.topLeft());
        painter.drawPoint(scaled_bbox.bottomRight());
        painter.drawPoint(scaled_bbox.center());
        painter.drawPoint(scaled_bbox.topRight());
        painter.drawPoint(scaled_bbox.bottomLeft());
    }

    painter.end();

}

void ImageLabel::mousePressEvent(QMouseEvent *ev){

    if(base_pixmap.isNull()) return;

    QPoint image_location = getScaledImageLocation(ev->pos());

    if (bbox_state == WAIT_START)
        current_mode = checkMode(image_location);

    if(current_mode == MODE_SELECT && ev->button() == Qt::LeftButton){

        // update label info panel
        emit selectLabel(*editbbox);

        if (selected && bbox_state == WAIT_START && same_box) {
            // Modify selected box

            // Timer before release event
            // to simulate click and hold
            dragTimer->setSingleShot(true);
            dragTimer->start(100);
            hold_status = true;

            // Anchor regions
            QPoint topLeft = editbbox->rect.topLeft();
            QPoint topRight = editbbox->rect.topRight();
            QPoint bottomLeft = editbbox->rect.bottomLeft();
            QPoint bottomRight = editbbox->rect.bottomRight();
            QPoint center = editbbox->rect.center();
            QRect topLeftProxmity(topLeft.rx() - 5, topLeft.ry() - 5, 10, 10);
            QRect topRightProxmity(topRight.rx() - 5, topLeft.ry() - 5, 10, 10);
            QRect bottomLeftProxmity(bottomLeft.rx() - 5, bottomRight.ry() - 5, 10, 10);
            QRect bottomRightProxmity(bottomRight.rx() - 5, bottomRight.ry() - 5, 10, 10);
            QRect centerProxmity(center.rx() - 5, center.ry() - 5, 10, 10);

            if(topLeftProxmity.contains(image_location)) {
                region =  TOP_LEFT;
                setCursor(Qt::SizeFDiagCursor);
            }
            else if(topRightProxmity.contains(image_location)) {
                region = TOP_RIGHT;
                setCursor(Qt::SizeBDiagCursor);
            }
            else if(bottomLeftProxmity.contains(image_location)) {
                region = BOTTOM_LEFT;
                setCursor(Qt::SizeBDiagCursor);
            }
            else if(bottomRightProxmity.contains(image_location)) {
                region = BOTTOM_RIGHT;
                setCursor(Qt::SizeFDiagCursor);
            }
            else if(centerProxmity.contains(image_location)){
                region = CENTER;
                setCursor(Qt::SizeAllCursor);
            }
        }else if (!selected && bbox_state == WAIT_START) {
            // select box from image
            drawEditLabel(*editbbox);
            selected = true;
            selected_bbox = *editbbox;

            // selected box prioritized in list for editing
            BoundingBox temp = bboxes[0];
            bboxes[0] = bboxes[editbbox_position];
            bboxes[editbbox_position] = temp;

        }else if (selected && bbox_state == WAIT_START && !same_box){
            // handle removing selection if you click into a different
            // bounding box
            region = NONE;
            selected = false;
            emit deselectLabel();
            drawLabel();
        }

    }else if(current_mode == MODE_SELECT && ev->button() == Qt::RightButton && same_box)
    {
        // label class and attribute editing
        if(selected){
            inputDialog->box = *editbbox;
            inputDialog->load(true);
            inputDialog->exec();

            if(inputDialog->box.id == -1){

                qDebug() << "No ID is entered";

            }else{
                // Update className, id and attributes
                editbbox->classname = inputDialog->box.classname;
                editbbox->id = inputDialog->box.id;
                editbbox->attributes = inputDialog->box.attributes;
                updateLabel(*editbbox);
                selected = false;
                emit deselectLabel();
                drawLabel();
            }
        }

    }else if(current_mode == MODE_DRAW && ev->button() == Qt::RightButton && bbox_state == DRAWING_BBOX){
        // Cancel drawing the box
        drawLabel();
        bbox_state = WAIT_START;

    }else if(ev->button() == Qt::LeftButton && selected == true && !same_box){
        // Cancel selection upon click outside box
        region = NONE;
        selected = false;
        emit deselectLabel();
        drawLabel();

    }else if(current_mode == MODE_DRAW && ev->button() == Qt::LeftButton){
        // Initiate box creation mode
        if(bbox_state == WAIT_START){

            dragTimer->setSingleShot(true);
            dragTimer->start(100);
            hold_status = true;

            bbox_origin = image_location;

            bbox_state = DRAWING_BBOX;
        }
    }
}

QRect ImageLabel::clip(QRect bbox){

    auto xpad = (width() - scaled_width)/2;

    bbox.setLeft(std::max(xpad, bbox.left()));
    bbox.setRight(std::min(width()-xpad, bbox.right()));

    auto ypad = (height() - scaled_height)/2;

    bbox.setTop(std::max(ypad, bbox.top()));
    bbox.setBottom(std::min(height()-ypad, bbox.bottom()));

    return bbox;
}

void ImageLabel::mouseReleaseEvent(QMouseEvent *ev){

    // TODO (Kamal) Refactor this
        setCursor(Qt::CrossCursor);

        if (dragTimer->isActive() && current_mode == MODE_DRAW){
            drawLabel();
            hold_status = false;
            bbox_state = WAIT_START;
            return;
        }else if(dragTimer->isActive() && current_mode == MODE_SELECT){
            hold_status = false;
            bbox_state = WAIT_START;
            return;
        }else{
            dragTimer->stop();
            hold_status = true;
        }

        if(current_mode == MODE_DRAW && bbox_state == DRAWING_BBOX && hold_status == true){
            // drawing finished
            hold_status = false;
            QPoint image_location = getScaledImageLocation(ev->pos());
            bbox_final = image_location;

            QRect new_box = QRect(bbox_origin, bbox_final).normalized();

            //create ID dialog
            if(new_box.width() > 10 && new_box.height() > 10){

                inputDialog->load(false);
                inputDialog->exec();

                if(inputDialog->status){
                    if(inputDialog->box.id == -1){
                        QMessageBox msgBox;
                        msgBox.setText("No ID entered");
                        msgBox.exec();
                    }else{
                        addLabel(new_box, inputDialog->box);
                    }
                }
            }
            drawLabel();
            bbox_state = WAIT_START;
        }else if(current_mode == MODE_SELECT){

            if(selected && bbox_state == DRAWING_BBOX && hold_status == true){
                hold_status = false;
                dragTimer->stop();
                bbox_state = WAIT_START;
                region = NONE;
                selected_bbox = *editbbox;
                updateLabel(*editbbox);
                bool found = searchBboxes(selected_bbox, bboxes, editbbox_position);
                if (found) {
                   BoundingBox temp = bboxes[0];
                   bboxes[0] = bboxes[editbbox_position];
                   bboxes[editbbox_position] = temp;
                }

            }else if(selected && bbox_state == WAIT_START) {
                hold_status = false;
                ev->ignore();
            }else{
                hold_status = false;
                ev->ignore();
            }

        }else{
            hold_status = false;
            dragTimer->stop();
            bbox_state = WAIT_START;
            drawLabel();
        }
}

void ImageLabel::mouseMoveEvent(QMouseEvent *ev){

    if(base_pixmap.isNull()) return;

    if(current_mode == MODE_DRAW && bbox_state == DRAWING_BBOX && hold_status == true){

        scaledPixmap();
        QPoint image_location = getScaledImageLocation(ev->pos());
        createbbox.rect.setTopLeft(bbox_origin);
        createbbox.rect.setBottomRight(image_location);
        drawBoundingBox(createbbox, Qt::blue, MODE_DRAW);
        QLabel::setPixmap(scaled_pixmap);
        bbox_state = DRAWING_BBOX;

    }else if(current_mode == MODE_SELECT && selected && hold_status == true){
        // Box Edit mode

            scaledPixmap();
            QPoint image_location = getScaledImageLocation(ev->pos());

        if(region == TOP_LEFT){
            // Top left anchor selected for resizing

            editbbox->rect.setTopLeft(image_location);
            editbbox->rect.setBottomRight(editbbox->rect.bottomRight());

        }if(region == TOP_RIGHT){
            // Top right anchor selected for resizing

            editbbox->rect.setTopRight(image_location);
            editbbox->rect.setBottomLeft(editbbox->rect.bottomLeft());

        }else if(region == BOTTOM_LEFT) {
            // Bottom left anchor selected for resizing

            editbbox->rect.setTopRight(editbbox->rect.topRight());
            editbbox->rect.setBottomLeft(image_location);

        }else if(region == BOTTOM_RIGHT) {
            // Botton right anchor selected for resizing

            editbbox->rect.setTopLeft(editbbox->rect.topLeft());
            editbbox->rect.setBottomRight(image_location);

        }else if(region == CENTER){
            // Center anchor selected for movement

            editbbox->rect.moveCenter(image_location);

        }

        // Show updated coordinates on canvas
        if(region != NONE){
            drawBoundingBox(*editbbox, Qt::green, MODE_SELECT);
            QLabel::setPixmap(scaled_pixmap);
            bbox_state = DRAWING_BBOX;
        }
    }
}

void ImageLabel::drawBoundingBox(BoundingBox bbox){
    // Choose random color for a class
    srand(static_cast<unsigned int>(bbox.classid + 1));
    QColor colour(rand()%255, rand()%255, rand()%255);
    drawBoundingBox(bbox, colour);
}

void ImageLabel::drawBoundingBox(BoundingBox bbox, QColor colour){
    // Draw Bounding Box with a specific color

    if(scaled_pixmap.isNull()) return;

    QPainter painter;
    painter.begin(&scaled_pixmap);
    QPen pen(colour, 2);
    painter.setPen(pen);

    auto scaled_bbox = bbox.rect;

    scaled_bbox.setRight(static_cast<int>(scaled_bbox.right() * scale_factor));
    scaled_bbox.setLeft(static_cast<int>(scaled_bbox.left() * scale_factor));
    scaled_bbox.setTop(static_cast<int>(scaled_bbox.top() * scale_factor));
    scaled_bbox.setBottom(static_cast<int>(scaled_bbox.bottom() * scale_factor));

    if(bbox.classname != ""){

        painter.setFont(QFont("Helvetica", 10));
        painter.drawText(scaled_bbox.bottomLeft(), bbox.classname);
        painter.drawText(scaled_bbox.topRight(), QString::number(bbox.id));

    }

    painter.drawRect(scaled_bbox);
    colour.setAlpha(50);
    painter.setBrush(QBrush(colour));
    painter.fillRect(scaled_bbox, painter.brush());

    painter.end();

}

void ImageLabel::setBoundingBoxes(QList<BoundingBox> input_bboxes){
    // populate bounding box list
    bboxes.clear();
    bboxes = input_bboxes;
    if(!selected)
        drawLabel();
}

void ImageLabel::updateLabel(BoundingBox box)
{
    // update Label in both screen and database
    emit updateLabel(original_box, box);
}

void ImageLabel::setScaledContents(bool should_scale){
    shouldScaleContents = should_scale;

    if(!shouldScaleContents){
        scale_factor = 1.0;
    }

}

bool ImageLabel::scaleContents(void){
    return shouldScaleContents;
}

void ImageLabel::drawLabel(){
    // draw all bounding boxes

    scaledPixmap();

    if(scale_factor == 1.0){
        scaled_pixmap = base_pixmap;
    }

    BoundingBox bbox;
    foreach(bbox, bboxes)
            drawBoundingBox(bbox);

    QLabel::setPixmap(scaled_pixmap);
}

void ImageLabel::addLabel(QRect rect, BoundingBox bbox){
    // add label to screen and database
    BoundingBox new_bbox;
    new_bbox.rect = rect;
    new_bbox.id = bbox.id;
    new_bbox.classname = bbox.classname;
    new_bbox.attributes = bbox.attributes;

    bboxes.append(new_bbox);
    emit newLabel(new_bbox);

    drawLabel();
}

void ImageLabel::addLabel(BoundingBox box){
    // seems redundant
    emit newLabel(box);

    drawLabel();
}

void ImageLabel::keyPressEvent(QKeyEvent *event)
{

    if(selected && (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)){
        // Remove label using delete or backspace
        selected = false;
        emit removeLabel(*editbbox);
        emit deselectLabel();
    }else if(current_mode == MODE_DRAW && event->key() == Qt::Key_Escape && bbox_state == DRAWING_BBOX){
        // Cancel label drawing on Escape key
        drawLabel();
        bbox_state = WAIT_START;
    }else if(current_mode == MODE_SELECT && selected == true && event->key() == Qt::Key_Escape){
        // Cancel label selection  on Escape key
        emit deselectLabel();
        selected = false;
        drawLabel();
    }else{
        event->ignore();
    }
}
