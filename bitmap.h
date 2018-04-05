#ifndef BITMAP_H_
#define BITMAP_H_

#include <string>
#include <vector>
#include <fstream>
#include <utility>
#include <algorithm>
#include <stdexcept>

struct Color24 {
  Color24() { r = g = b = 0; }
  Color24(uint8_t _r, uint8_t _g, uint8_t _b) : b(_b), g(_g), r(_r) {}
  Color24(uint32_t i) {
    uint8_t* t = (uint8_t*)&i;
    b = *(t++);
    g = *(t++);
    r = *t;
  }
  uint8_t b, g, r;
  operator uint32_t() {
    int tr = 0;
    uint8_t* t = (uint8_t*)&tr;
    *(t++) = b;
    *(t++) = g;
    *t = r;
    return tr;
  }
};

template <class Color> class Bitmap {
public:
  typedef unsigned int size_type;
  typedef std::pair<size_type, size_type> SizeXY;

private:
  class Row {
    std::vector<std::vector<Color>>& b_;
    size_type r_;
  public:
    Row(std::vector<std::vector<Color>>& b, size_type r) : b_(b), r_(r) {}
    Color& operator[](size_type z) const { return b_[z][r_]; }
  };
  class RowConst {
    const std::vector<std::vector<Color>>& b_;
    size_type r_;
  public:
    RowConst(const std::vector<std::vector<Color>>& b, size_type r) : b_(b), r_(r) {}
    const Color& operator[](size_type z) const { return b_[z][r_]; }
  };

public:
  Bitmap() : height_(0), width_(0) { ; }
  Bitmap(size_type w, size_type h) :
      bitmap_(h, std::vector<Color>(w)), height_(h), width_(w) {}
  Bitmap(const Bitmap& bmp) :
      bitmap_(bmp.bitmap_), height_(bmp.height_), width_(bmp.width_) {}
  Bitmap(Bitmap&& bmp) : height_(bmp.height_), width_(bmp.width_) {
    bitmap_.swap(bmp.bitmap_);
  }

  const Bitmap& operator=(const Bitmap& bmp) {
    bitmap_ = bmp.bitmap_;
    height_ = bmp.height_;
    width_ = bmp.width_;
    return *this;
  }
  const Bitmap& operator=(Bitmap&& bmp) {
    bitmap_.swap(bmp.bitmap_);
    height_ = bmp.height_;
    width_ = bmp.width_;
    return *this;
  }

  Row operator[](size_type x) { return Row(bitmap_, x); }
  RowConst operator[](size_type x) const { return RowConst(bitmap_, x); }
  const Color& at(size_type x, size_type y) const {
    if (x >= width_ || y >= height_)
      throw std::out_of_range("Index out of range");
    return bitmap_[y][x];
  }
  Color& at(size_type x, size_type y) {
    if (x >= width_ || y >= height_)
      throw std::out_of_range("Index out of range");
    return bitmap_[y][x];
  }

  SizeXY size() const {
    return SizeXY(width_, height_);
  }
  size_type Width() const { return width_; }
  size_type Height() const { return height_; }

  void resize(size_type w, size_type h) {
    bitmap_.resize(h);
    for (size_type i = 0; i < h; i++) bitmap_[i].resize(w);
    height_ = h, width_ = w;
  }
  void Fill(Color c) {
    size_type i, j;
    for (i = 0; i < height_; i++)
      for (j = 0; j < width_; j++) bitmap_[i][j] = c;
  }
  void Fill(size_type xa, size_type ya, size_type xb, size_type yb, Color c) {
    if (xa > xb) std::swap(xa, xb);
    if (ya > yb) std::swap(ya, yb);
    if (xa >= width_ || ya >= height_) return;
    if (xb > width_) xb = width_;
    if (yb > height_) yb = height_;
    size_type i, j;
    for (i = ya; i < yb; i++)
      for (j = xa; j < xb; j++) bitmap_[i][j] = c;
  }
  void Point(size_type x, size_type y, Color c, size_type size = 1) {
    x -= (size - 1) >> 1;
    y -= (size - 1) >> 1;
    Fill(x, y, x + size, y + size, c);
  }
  void LineX(size_type x, size_type ya, size_type yb, Color c,
      size_type size = 1) {
    if (ya > yb) std::swap(ya, yb);
    if (size == 1) {
      for (size_type i = ya; i < yb; i++) bitmap_[i][x] = c;
    } else {
      size_type i, j, xa;
      if (x < (size >> 1)) {
        xa = 0;
        x += size >> 1;
      } else {
        xa = x - (size >> 1);
        x = xa + size;
      }
      x = xa + size;
      if (x > width_) x = width_;
      if (yb > height_) yb = height_;
      for (i = ya; i < yb; i++)
        for (j = xa; j < x; j++) bitmap_[i][j] = c;
    }
  }
  void LineY(size_type xa, size_type xb, size_type y, Color c,
      size_type size = 1) {
    if (xa > xb) std::swap(xa, xb);
    size_type i, j, ya;
    if (y < (size >> 1)) {
      ya = 0;
      y += size >> 1;
    } else {
      ya = y - (size >> 1);
      y = ya + size;
    }
    if (xb > width_) xb = width_;
    if (y > height_) y = height_;

    for (i = ya; i < y; i++)
      for (j = xa; j < xb; j++) bitmap_[i][j] = c;
  }
  void Insert(size_type x, size_type y, Bitmap& c) {
    size_type i, il, j, jl;
    size_type xm = std::min(width_, c.width_ + x);
    size_type ym = std::min(height_, c.height_ + y);
    for (i = y, il = 0; i < ym; i++, il++)
      for (j = x, jl = 0; j < xm; j++, jl++) bitmap_[i][j] = c.bitmap_[il][jl];
  }
  void Trim(size_type xa, size_type ya, size_type xb, size_type yb) {
    if (xa > xb) std::swap(xa, xb);
    if (ya > yb) std::swap(ya, yb);
    if (xa >= width_ || ya >= height_) return;
    if (xb > width_) xb = width_;
    if (yb > height_) yb = height_;
    height_ = yb - ya;
    width_ = xb - xa;
    bitmap_.erase(bitmap_.begin() + yb, bitmap_.end());
    bitmap_.erase(bitmap_.begin(), bitmap_.begin() + ya);
    for (size_type i = 0; i < height_; i++) {
      bitmap_[i].erase(bitmap_[i].begin() + xb, bitmap_[i].end());
      bitmap_[i].erase(bitmap_[i].begin(), bitmap_[i].begin() + xa);
    }
  }

  void Read(std::string filename);
  void ToBMP(std::string filename) const;

protected:
  std::vector<std::vector<Color>> bitmap_;
  size_type height_, width_;
  constexpr bool IsBigEndian() const {
    int i = 1;
    return !*((char*)&i);
  };
  template <class T> void Write_(std::ostream& o, T value) const {
    if (IsBigEndian()) {
      const char* ptr = (const char*)&value + sizeof(T);
      for (size_type i = 0; i < sizeof(T); i++) o.write((const char*)&--ptr, 1);
    } else {
      o.write((const char*)&value, sizeof(T));
    }
  }
  template <class T> void Read_(std::istream& i, T& value) const {
    char t[sizeof(T)];
    i.read(t, sizeof(T));
    if (IsBigEndian()) {
      for (size_type i = 0; i < sizeof(T); i++)
        ((char*)&value)[i] = t[i];
    } else {
      for (size_type i = 0; i < sizeof(T); i++)
        ((char*)&value)[sizeof(T) - i - 1] = t[i];
    }
  }
};

template <> void Bitmap<Color24>::Read(std::string filename) {
  std::fstream fin(filename, std::ios_base::in | std::ios_base::binary);
  if (!fin.is_open()) return;
  char tc;
  int16_t ts;
  int32_t ti;
  int64_t tll;
  size_type w, h, bdo, bhs;
  Read_(fin, ts);
  Read_(fin, tll);
  Read_(fin, bdo);
  Read_(fin, bhs);
  Read_(fin, w);
  Read_(fin, h);
  Read_(fin, ts);
  Read_(fin, ts);
  if (ts != 24) return;
  Read_(fin, ti);
  if (ti) return;
  Read_(fin, tll);
  Read_(fin, tll);
  Read_(fin, ti);
  size_type i, j, tw = ((~(3 * w) & 3) + 1) & 3;
  for (i = 54; i < bdo; i++) Read_(fin, tc);
  resize(w, h);
  for (i = height_ - 1; ~i; i--) {
    for (j = 0; j < width_; j++) {
      Read_(fin, bitmap_[i][j].b);
      Read_(fin, bitmap_[i][j].g);
      Read_(fin, bitmap_[i][j].r);
    }
    for (j = 0; j < tw; j++) Read_(fin, tc);
  }
}
template <> void Bitmap<Color24>::ToBMP(std::string filename) const {
  std::fstream fout(filename, std::ios_base::out | std::ios_base::binary);
  Write_(fout, (int16_t)19778);
  Write_(fout, 54 + 3 * height_ * width_);
  Write_(fout, 0);
  Write_(fout, 54);
  Write_(fout, 40);
  Write_(fout, width_);
  Write_(fout, height_);
  Write_(fout, (int64_t)1572865);
  Write_(fout, 3 * height_ * width_);
  Write_(fout, 0);
  Write_(fout, 0);
  Write_(fout, 0);
  Write_(fout, 0);
  size_type i, j, tw = ((~(3 * width_) & 3) + 1) & 3;
  for (i = height_ - 1; ~i; i--) {
    for (j = 0; j < width_; j++)
      fout << bitmap_[i][j].b << bitmap_[i][j].g << bitmap_[i][j].r;
    for (j = 0; j < tw; j++) fout << '\0';
  }
  fout.close();
}


typedef Bitmap<Color24> Bitmap24;

#endif
