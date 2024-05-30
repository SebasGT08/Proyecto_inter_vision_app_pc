#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

// Función para cambiar el contenido de las gafas con el feed de la cámara web usando operaciones bitwise
void cambiarContenidoGafas(Mat& imagen, const Mat& mascara, const Mat& contenido) {
    Mat contenidoRedimensionado;
    resize(contenido, contenidoRedimensionado, imagen.size());

    // Crear una imagen del mismo tamaño que la imagen original con el contenido de la cámara web
    Mat gafasContenido = Mat::zeros(imagen.size(), imagen.type());
    contenidoRedimensionado.copyTo(gafasContenido, mascara);

    // Aplicar la máscara inversa a la imagen original para mantener los otros colores
    Mat imagenFondo;
    bitwise_and(imagen, imagen, imagenFondo, ~mascara);

    // Combinar las dos imágenes
    bitwise_or(gafasContenido, imagenFondo, imagen);
}

int main(int argc, char* argv[]) {
    // Manejo de Video
    VideoCapture videoPrincipal("http://192.168.83.169:5000/video_feed");
    VideoCapture videoCamara(0); // Cámara web de la computadora

    // Verificamos si los streams de video se pudieron abrir
    if (!videoPrincipal.isOpened() || !videoCamara.isOpened()) {
        cerr << "Error al abrir el stream de video!" << endl;
        return -1;
    }

    // Crear ventanas para mostrar los resultados
    namedWindow("Original", WINDOW_AUTOSIZE);
    namedWindow("Gafas Contenido Cambiado", WINDOW_AUTOSIZE);
    namedWindow("HSV", WINDOW_AUTOSIZE); // Ventana de depuración

    Mat frameColor;
    Mat imgHSV;
    Mat mascara;
    Mat frameCamara;

    // Valores HSV para detectar el color magenta
    int hmin = 140, hmax = 170, smin = 100, smax = 255, vmin = 150, vmax = 255;

    // Bucle principal
    while (true) {
        videoPrincipal >> frameColor; // Capturar frame del video principal
        videoCamara >> frameCamara;   // Capturar frame de la cámara web
        if (frameColor.empty() || frameCamara.empty()) break; // Salir si no hay más frames

        // Convertir a HSV
        cvtColor(frameColor, imgHSV, COLOR_BGR2HSV);

        // Crear una máscara basada en el rango HSV
        inRange(imgHSV, Scalar(hmin, smin, vmin), Scalar(hmax, smax, vmax), mascara);

        // Cambiar el contenido de las gafas en la imagen
        Mat gafasContenidoCambiado = frameColor.clone();
        cambiarContenidoGafas(gafasContenidoCambiado, mascara, frameCamara);

        // Mostrar las imágenes en las ventanas
        imshow("Original", frameColor);
        imshow("Gafas Contenido Cambiado", gafasContenidoCambiado);
        imshow("HSV", imgHSV); // Mostrar la imagen HSV para depuración

        // Salir si se presiona la tecla ESC
        if (waitKey(1) == 27) break;
    }

    // Liberar los videos y destruir todas las ventanas
    videoPrincipal.release();
    videoCamara.release();
    destroyAllWindows();

    return 0;
}
