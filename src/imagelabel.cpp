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

    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    setDrawMode();
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

void ImageLabel::setDrawMode(){
    current_mode = MODE_DRAW;
    setCursor(Qt::CrossCursor);
    rubberBand->setGeometry(QRect(bbox_origin, QSize()));
    rubberBand->show();
}

void ImageLabel::setDrawDragMode(){
    current_mode = MODE_DRAW_DRAG;
    rubberBand->setGeometry(QRect(bbox_origin, QSize()));
    rubberBand->show();
}

void ImageLabel::setSelectMode(){
    current_mode = MODE_SELECT;
    setCursor(Qt::ArrowCursor);
    rubberBand->hide();
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

interactionState ImageLabel::checkMode(QPoint image_location)
{
    scaledPixmap();

    if(scale_factor == 1.0){
        scaled_pixmap = base_pixmap;
    }

    BoundingBox bbox;
    for (int i = 0; i<bboxes.size(); i++) {
        if (bboxes[i].rect.contains(image_location)){
            editbbox = &bboxes[i];
            original_box = bboxes[i];
            return interactionState::MODE_SELECT;
        }
    }
    return interactionState::MODE_DRAW;
}

void ImageLabel::drawEditLabel(BoundingBox bbox){
    scaledPixmap();
    drawBoundingBox(bbox, Qt::green, MODE_SELECT);
    QLabel::setPixmap(scaled_pixmap);
}

void ImageLabel::drawBoundingBox(BoundingBox bbox, QColor colour, interactionState mode){

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

        //painter.fillRect(QRect(scaled_bbox.bottomLeft(), scaled_bbox.bottomRight()+QPoint(0,-10)).normalized(), QBrush(Qt::white));

        painter.setFont(QFont("Helvetica", 10));
        painter.drawText(scaled_bbox.bottomLeft(), bbox.classname);

    }
    painter.drawRect(scaled_bbox);

    if(mode == MODE_SELECT){
        pen.setWidth(5);
        painter.setPen(pen);
        painter.drawPoint(scaled_bbox.topLeft());
        painter.drawPoint(scaled_bbox.bottomRight());
        painter.drawPoint(scaled_bbox.center());
    }

    painter.end();

}


void ImageLabel::mousePressEvent(QMouseEvent *ev){

    if(base_pixmap.isNull()) return;

    QPoint image_location = getScaledImageLocation(ev->pos());

    if (bbox_state == WAIT_START)
        current_mode = checkMode(image_location);

    if(current_mode == MODE_SELECT && ev->button() == Qt::LeftButton){

        emit selectLabel(*editbbox);
        if (selected && bbox_state == WAIT_START) {

            QPoint topLeft = editbbox->rect.topLeft();
            QPoint bottomRight = editbbox->rect.bottomRight();
            QPoint center = editbbox->rect.center();
            QRect topLeftProxmity(topLeft.rx() - 10, topLeft.ry() - 10, 20, 20);
            QRect bottomRightProxmity(bottomRight.rx() - 10, bottomRight.ry() - 10, 20, 20);
            QRect centerProxmity(center.rx() - 10, center.ry() - 10, 20, 20);

            if(topLeftProxmity.contains(image_location)) {

                region =  TOP_LEFT;
                std::cout<<"near top left"<<std::endl;

            }
            else if(bottomRightProxmity.contains(image_location)) {

                region = BOTTOM_RIGHT;
                std::cout<<"near bottom right"<<std::endl;

            }
            else if(centerProxmity.contains(image_location)){

                region = CENTER;
                std::cout<<"near center"<<std::endl;

            }
        }else if(selected && bbox_state == DRAWING_BBOX){

            std::cout<<"finished editing"<<std::endl;
            bbox_state = WAIT_START;
            selected = false;
            region = NONE;
            updateLabel(*editbbox);
            drawLabel();

        }else {
            drawEditLabel(*editbbox);
            selected = true;
        }

    }else if(current_mode == MODE_SELECT && ev->button() == Qt::RightButton)
    {
        bool ok;
        QString current_classname = QInputDialog::getText(0, "Create new ID", "New ID: ",
                                                 QLineEdit::Normal, "", &ok);

        if(current_classname == ""){

            qDebug() << "No class selected!";

        }else{
            editbbox->classname = current_classname;
            updateLabel(*editbbox);

        }
    }else if(current_mode == MODE_DRAW && ev->button() == Qt::RightButton && bbox_state == DRAWING_BBOX){
            drawLabel();
            bbox_state = WAIT_START;
    }else if(ev->button() == Qt::LeftButton && selected == true){
        selected = false;
        drawLabel();
    }else if(current_mode == MODE_DRAW && ev->button() == Qt::LeftButton){
        if(bbox_state == WAIT_START){

            bbox_origin = image_location;

            bbox_state = DRAWING_BBOX;

        }else if(bbox_state == DRAWING_BBOX){

            bbox_final = image_location;
            //create ID dialog
            bool ok;
            QString current_classname = QInputDialog::getText(0, "Create new ID", "New ID: ",
                                                 QLineEdit::Normal, "", &ok);

            QRect new_box = QRect(bbox_origin, bbox_final).normalized();
            //new_box = clip(new_box);

            if(new_box.width() > 0 && new_box.height() > 0){

                if(ok){
                    if(current_classname == ""){
                        QMessageBox msgBox;
                        msgBox.setText("Class not chosen");
                        msgBox.exec();
                    }else{
                        addLabel(new_box, current_classname);
                    }
                }
        }
    drawLabel();
    bbox_state = WAIT_START;
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
    /*if(current_mode == MODE_DRAW_DRAG){
        bbox_final = ev->pos();

        QRect bbox(bbox_origin, bbox_final);
        rubberBand->setGeometry(clip(bbox.normalized()));

        bbox_state = WAIT_START;
    }else{
        ev->ignore();
    }*/
    ev->ignore();
}

void ImageLabel::mouseMoveEvent(QMouseEvent *ev){

    if(base_pixmap.isNull()) return;

    if(current_mode == MODE_DRAW && bbox_state == DRAWING_BBOX ){
        scaledPixmap();
        QPoint image_location = getScaledImageLocation(ev->pos());
        createbbox.rect.setTopLeft(bbox_origin);
        createbbox.rect.setBottomRight(image_location);
        drawBoundingBox(createbbox, Qt::blue, MODE_DRAW);
        QLabel::setPixmap(scaled_pixmap);
        bbox_state = DRAWING_BBOX;
        std::cout<<"box creation"<<std::endl;
    }else if(current_mode == MODE_SELECT && selected){
        if(region == TOP_LEFT){

            scaledPixmap();
            QPoint image_location = getScaledImageLocation(ev->pos());
            std::cout<<original_box.rect.x()<<std::endl;
            editbbox->rect.setTopLeft(image_location);
            editbbox->rect.setBottomRight(editbbox->rect.bottomRight());
            std::cout<<editbbox->rect.x()<<std::endl;
            drawBoundingBox(*editbbox, Qt::green, MODE_SELECT);
            QLabel::setPixmap(scaled_pixmap);
            bbox_state = DRAWING_BBOX;
            std::cout<<"resizing top left"<<std::endl;

        }else if(region == BOTTOM_RIGHT) {

            scaledPixmap();
            QPoint image_location = getScaledImageLocation(ev->pos());
            editbbox->rect.setTopLeft(editbbox->rect.topLeft());
            editbbox->rect.setBottomRight(image_location);
            drawBoundingBox(*editbbox, Qt::green, MODE_SELECT);
            QLabel::setPixmap(scaled_pixmap);
            bbox_state = DRAWING_BBOX;
            std::cout<<"resizing bottom right"<<std::endl;

        }else if(region == CENTER){

            scaledPixmap();
            QPoint image_location = getScaledImageLocation(ev->pos());
            editbbox->rect.moveCenter(image_location);
            drawBoundingBox(*editbbox, Qt::green, MODE_SELECT);
            QLabel::setPixmap(scaled_pixmap);
            bbox_state = DRAWING_BBOX;
            std::cout<<"moving center"<<std::endl;

        }

    }
}

void ImageLabel::drawBoundingBox(BoundingBox bbox){
    srand(static_cast<unsigned int>(bbox.classid));
    QColor colour(rand()%255, rand()%255, rand()%255);
    drawBoundingBox(bbox, colour);
}

void ImageLabel::drawBoundingBox(BoundingBox bbox, QColor colour){

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

        //painter.fillRect(QRect(scaled_bbox.bottomLeft(), scaled_bbox.bottomRight()+QPoint(0,-10)).normalized(), QBrush(Qt::white));

        painter.setFont(QFont("Helvetica", 10));
        painter.drawText(scaled_bbox.bottomLeft(), bbox.classname);

    }

    painter.drawRect(scaled_bbox);

    painter.end();

}

void ImageLabel::setBoundingBoxes(QList<BoundingBox> input_bboxes){
    bboxes.clear();
    bboxes = input_bboxes;
    drawLabel();
}

void ImageLabel::updateLabel(BoundingBox box)
{
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

    scaledPixmap();

    if(scale_factor == 1.0){
        scaled_pixmap = base_pixmap;
    }

    BoundingBox bbox;
    foreach(bbox, bboxes)
            drawBoundingBox(bbox);

    QLabel::setPixmap(scaled_pixmap);
}

void ImageLabel::addLabel(QRect rect, QString classname){
    BoundingBox new_bbox;
    new_bbox.classname = classname;
    new_bbox.rect = rect;

    bboxes.append(new_bbox);
    emit newLabel(new_bbox);

    drawLabel();
}

void ImageLabel::addLabel(BoundingBox box){
    emit newLabel(box);

    drawLabel();
}

void ImageLabel::keyPressEvent(QKeyEvent *event)
{

    if(selected && (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)){
        selected = false;
        emit removeLabel(*editbbox);
    }else if(current_mode == MODE_DRAW && event->key() == Qt::Key_Escape && bbox_state == DRAWING_BBOX){
            drawLabel();
            bbox_state = WAIT_START;
    }else if(current_mode == MODE_SELECT && selected == true && event->key() == Qt::Key_Escape){
            selected = false;
            drawLabel();
    }else{
        event->ignore();
    }
}
