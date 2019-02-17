#include "RandomTextColor.hpp"
#include <cassert>
#include <vector>
#include <algorithm>
#include <boost/polygon/voronoi.hpp>
#include <array>
#include <list>

#ifndef _DEBUG
#ifndef NDEBUG
#define NDEBUG (1u)
#endif
#endif

#ifndef error_return
#define error_return(...) return errorReturn(QStringLiteral(__VA_ARGS__))
#endif

RandomTextColor::RandomTextColor(const QString & a, const QString & b) :
    mmmInputFileName(a),
    mmmOutputFileName(b) {
}

constexpr const static int colormax = 65535;
constexpr const static double int64todouble_k = 1.0 / colormax;

inline std::vector< std::array< QPointF, 3 > > toRandomTriangle(int argWidth, int argHeight, int varStep = 3) {

    assert(varStep > 0);
    assert(argWidth > 6);
    assert(argHeight > 6);

    std::uniform_real_distribution<double> varXRandom(0.3, varStep - 0.3);
    std::mt19937 varEng{ std::random_device{}() };

    using Point = boost::polygon::point_data<double>;

    std::vector< Point > varVertices;
    varVertices.reserve(
        std::max(std::max(argWidth, argHeight), std::max(1,
            std::max(1, argWidth / varStep)*
            std::max(1, argHeight / varStep))));

    for (int varX = 0; varX < argWidth; varX += varStep) {
        for (int varY = 0; varY < argHeight; varY += varStep) {
            varVertices.emplace_back(varX + varXRandom(varEng), varY + varXRandom(varEng));
        }
    }

    std::vector< std::array< QPointF, 3 > > varReturn;
    boost::polygon::voronoi_diagram<double> varAns;
    boost::polygon::construct_voronoi(varVertices.begin(), varVertices.end(), &varAns);

    for (const auto & varI : varAns.vertices()) {
        std::list< QPointF > varPolygon;
        auto varEdge = varI.incident_edge();
        do {
            auto varCell = varEdge->cell();
            const auto & varPoint =
                varVertices[varCell->source_index()];
            varPolygon.emplace_back(varPoint.x(), varPoint.y());
            if (varPolygon.size() > 2/*infact it is 3*/) {
                auto varFirst = varPolygon.begin();
                auto varSecond = varFirst; ++varSecond;
                auto varThird = varSecond; ++varThird;
                varReturn.push_back({ *varFirst, *varSecond, *varThird });
                varPolygon.erase(varSecond);
            }
            varEdge = varEdge->rot_next();
        } while (varEdge != varI.incident_edge());
    }

    return std::move(varReturn);

}

inline QImage toRandomTriangle(const QImage & arg) {

    auto varTriangles =
        toRandomTriangle(
            arg.width(), 
            arg.height(),
            4 /*晶格大小*/ );

    QImage varAns = arg.convertToFormat(QImage::Format_RGBA64);

    QPainter varPainter{ &varAns };
    varPainter.setPen(Qt::NoPen);

    varPainter.setRenderHints(QPainter::Antialiasing |
        QPainter::TextAntialiasing |
        QPainter::SmoothPixmapTransform |
        QPainter::HighQualityAntialiasing);

    const auto varMaxX = arg.width() - 1;
    const auto varMaxY = arg.height() - 1;

    for (const auto & varI : varTriangles) {

        int x, y;

        {
            auto varRX =
                varI[0].x()*0.3333 + varI[1].x()*0.3333 + varI[2].x()*0.3333;
            auto varRY =
                varI[0].y()*0.3333 + varI[1].y()*0.3333 + varI[2].y()*0.3333;
            x = std::min(varMaxX, static_cast<int>(varRX));
            y = std::min(varMaxY, static_cast<int>(varRY));
        }

        auto varColor = std::min(255,
            qGray( arg.pixel (x, y) ) + ( std::rand()&15)) ;
        varPainter.setBrush(QColor(varColor,varColor,varColor));

        assert(varI.size() == 3);
        varPainter.drawPolygon(varI.data(), 3);

    }

    return std::move(varAns);

}

class ColorInt {
public:
    std::uint16_t r;
    std::uint16_t g;
    std::uint16_t b;
    std::uint16_t a;
};

class Color {
public:
    double r;
    double g;
    double b;
    double a;
    inline Color() :
        r{ 0. },
        g{ 0. },
        b{ 0. },
        a{ 0. } {
    }
    inline Color(double ar, double ag, double ab, double aa) :
        r{ ar },
        g{ ag },
        b{ ab },
        a{ aa } {
    }
    inline Color(const ColorInt & arg) :
        r(arg.r*int64todouble_k),
        g(arg.g*int64todouble_k),
        b(arg.b*int64todouble_k),
        a(arg.a*int64todouble_k) {
    }
    inline operator ColorInt() const {
        auto varF = [](double a)->std::uint16_t {
            return static_cast<std::uint16_t>(std::clamp<double>(
                std::fma(a, colormax, 0.5), 0., colormax));
        };
        ColorInt varAns{
            varF(r),
            varF(g),
            varF(b),
            varF(a)
        };
        return varAns;
    }
};

class CurrentColor {
public:
    const ColorInt * const color;
    int const bytePerLine;
};

class CurrentAnsColor {
public:
    ColorInt * color;
    int const bytePerLine;
};

template<typename F>
inline QImage mix_image(
    const QImage & varBasic,
    const QImage & varTarget,
    F && varFunction) {

    assert(varBasic.width() == varTarget.width());
    assert(varBasic.height() == varTarget.height());
    assert(varBasic.format() == varTarget.format());
    assert(varBasic.format() == QImage::Format_RGBA64);

    const auto varImageWidth = varBasic.width();
    const auto varImageHeight = varBasic.height();

    QImage varAns{
        varImageWidth,
        varImageHeight,
        QImage::Format_RGBA64
    };
    varAns.fill(QColor(0, 0, 0, 0));

    const auto varBPL_0 = varBasic.bytesPerLine();
    const auto varBPL_1 = varTarget.bytesPerLine();
    const auto varBPL_2 = varAns.bytesPerLine();

    auto varRawBasicBegin = varBasic.constBits();
    auto varRawTargetBegin = varTarget.constBits();
    auto varAnsBegin = varAns.bits();

    auto varBasicLineBegin = varRawBasicBegin;
    auto varTargetLineBegin = varRawTargetBegin;
    auto varAnsLineBegin = varAnsBegin;

    for (int varCurrentY = 0; varCurrentY < varImageHeight;
        ++varCurrentY,
        varBasicLineBegin += varBPL_0,
        varTargetLineBegin += varBPL_1,
        varAnsLineBegin += varBPL_2) {
        auto varPos0 = reinterpret_cast<const ColorInt *>(varBasicLineBegin);
        auto varPos1 = reinterpret_cast<const ColorInt *>(varTargetLineBegin);
        auto varPos2 = reinterpret_cast<ColorInt *>(varAnsLineBegin);
        for (int varCurrentX = 0; varCurrentX < varImageWidth;
            ++varCurrentX,
            ++varPos0,
            ++varPos1,
            ++varPos2) {
            varFunction(
                CurrentColor{ varPos0 ,varBPL_0 },
                CurrentColor{ varPos1 ,varBPL_1 },
                CurrentAnsColor{ varPos2 ,varBPL_2 },
                varImageWidth,
                varImageHeight,
                varCurrentX,
                varCurrentY);
        }
    }

    return std::move(varAns);

}


class RandomTextColorConvert;
template<>
class RandomTextColorPrivate<RandomTextColorConvert> {
    RandomTextColor * const super;
    QImage mmmInputImage;
    QImage mmmOutputImage;
    QImage mmmColorMask;
    std::size_t mmmImageWidth;
    std::size_t mmmImageHeight;
public:

    inline RandomTextColorPrivate(RandomTextColor * arg)
        :super(arg) {
    }

    inline bool errorReturn(const QString &) const {
        return false;
    }

    inline bool eval() {
        mmmInputImage = QImage{ super->mmmInputFileName };
        if (mmmInputImage.isNull()) {
            error_return("the image is empty");
        }

        /*open the image ...*/
        mmmImageWidth = static_cast<std::size_t>(
            mmmInputImage.width());
        mmmImageHeight = static_cast<std::size_t>(
            mmmInputImage.height());
        assert(mmmImageWidth > 0);
        assert(mmmImageHeight > 0);
        mmmInputImage = mmmInputImage.convertToFormat(
            QImage::Format_RGBA64
        );

        {
            auto varTmpImage = toRandomTriangle(mmmInputImage);
            mmmOutputImage =
                mix_image(mmmInputImage, varTmpImage,
                    [](
                        const CurrentColor & argInputColor0,
                        const CurrentColor & argInputColor1,
                        CurrentAnsColor argAnsColor,
                        int,
                        int,
                        int,
                        int) -> void {
                Color varInput0{ *(argInputColor0.color) };
                Color varInput1{ *(argInputColor1.color) };
                auto varF = [](double a, double b)->double {
                    const auto var = (std::rand()&255)/1024 + 0.5 ;
                    return std::clamp(a * (1 - var) + b * var, 0., 1.);
                };
                Color varAns{ varF(varInput0.r,varInput1.r),
                    varF(varInput0.g,varInput1.g),
                    varF(varInput0.b,varInput1.b),1 };
                *(argAnsColor.color) = varAns;
            });
            mmmInputImage = mmmOutputImage;
        }

        create_mask_image();
        mmmOutputImage = /*滤色*/
            mix_image(mmmInputImage, mmmColorMask,
                [](
                    const CurrentColor & argInputColor0,
                    const CurrentColor & argInputColor1,
                    CurrentAnsColor argAnsColor,
                    int,
                    int,
                    int,
                    int) -> void {
            Color varInput0{ *(argInputColor0.color) };
            Color varInput1{ *(argInputColor1.color) };
            auto varF = [](double a, double b)->double {
                return std::clamp(1. - (1. - a)*(1. - b), 0., 1.);
            };
            Color varAns{ varF(varInput0.r,varInput1.r),
                varF(varInput0.g,varInput1.g),
                varF(varInput0.b,varInput1.b),1 };
            *(argAnsColor.color) = varAns;
        });

        return mmmOutputImage.save(super->mmmOutputFileName);

    }

    inline void create_mask_image();

};

bool RandomTextColor::convert() {
    RandomTextColorPrivate<RandomTextColorConvert> var{ this };
    return var.eval();
}


inline void RandomTextColorPrivate<RandomTextColorConvert>::create_mask_image() {

    QImage varMaskImage = mmmInputImage;
    varMaskImage.fill(QColor(0, 0, 0, colormax));

    /* http://c.biancheng.net/view/639.html */
    std::uniform_int_distribution<> varXRandom(0, mmmInputImage.width() - 1);
    std::uniform_int_distribution<> varYRandom(0, mmmInputImage.height() - 1);
    std::uniform_int_distribution<> varRRandom(3,
        std::max(4, std::min(
            mmmInputImage.height() - 1,
            mmmInputImage.width() - 1) >> 2)
    );
    std::uniform_int_distribution<ushort>
        varRGBRanom(colormax >> 1, colormax);

    std::mt19937 varEng{ std::random_device{}() };

    QPainter varPainter{ &varMaskImage };
    varPainter.setPen(Qt::NoPen);

    varPainter.setRenderHints(QPainter::Antialiasing |
        QPainter::TextAntialiasing |
        QPainter::SmoothPixmapTransform |
        QPainter::HighQualityAntialiasing
    );

    for (int i = 0; i < 1024; ++i) {

        auto varRandomX = varXRandom(varEng);
        auto varRandomY = varYRandom(varEng);
        auto varRandomR = varRRandom(varEng);

        ushort varRGB[3]{
            varRGBRanom(varEng),
            0,
            varRGBRanom(varEng)
        };

        if (varRGB[0] > ((colormax >> 1) + (colormax >> 2))) {
            varRGB[1] = varRGB[0] - (colormax >> 2);
        } else {
            varRGB[1] = varRGB[0] + (colormax >> 2);
        }

        std::shuffle(std::begin(varRGB), std::end(varRGB), varEng);

        /*set random color*/
        varPainter.setBrush(QBrush(
            QColor::fromRgba64(
                varRGB[0]>>1,
                varRGB[1]>>1,
                varRGB[2]>>1
            )));

        /*draw random Ellipse*/
        varPainter.drawEllipse(
            QPoint{ varRandomX , varRandomY },
            varRandomR,
            varRandomR);

    }

    mmmColorMask = std::move(varMaskImage);

}









