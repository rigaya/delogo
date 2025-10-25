#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <utility>
#include <memory>

#include "module2.h"
#include "../delogo/logo.h"

struct LoadedLogo {
    std::wstring path;
    LOGO_HEADER logo_header;
    std::vector<uint8_t> ycc_rgba; // R=Y(0..255), G=Cb(0..255 offset +0.5), B=Cr(0..255 offset +0.5), A=255
    std::vector<uint8_t> dp_rgba;  // R=dp_y(0..255), G=dp_cb(0..255), B=dp_cr(0..255), A=255
};

LoadedLogo g_logo;

static inline double clamp(double v, double lo = 0.0, double hi = 255.0) {
    if (v < lo) v = lo; else if (v > hi) v = hi;
    return v;
}
static inline uint8_t clamp_u8(double v, double lo = 0.0, double hi = 255.0) {
    if (v < lo) v = lo; else if (v > hi) v = hi;
    return (uint8_t)(v + 0.5);
}

static int logo_file_header_ver(const LOGO_FILE_HEADER *logo_file_header) {
    int logo_header_ver = 0;
    if (0 == strcmp(logo_file_header->str, LOGO_FILE_HEADER_STR)) {
        logo_header_ver = 2;
    } else if (0 == strcmp(logo_file_header->str, LOGO_FILE_HEADER_STR_OLD)) {
        logo_header_ver = 1;
    }
    return logo_header_ver;
}

static void conv_logo_header_v1_to_v2(LOGO_HEADER *logo_header) {
    LOGO_HEADER_OLD old_header;
    memcpy(&old_header,       logo_header,      sizeof(old_header));
    memset(logo_header,       0,                sizeof(logo_header[0]));
    memcpy(logo_header->name, &old_header.name, sizeof(old_header.name));
    memcpy(&logo_header->x,   &old_header.x,    sizeof(short) * 8);
}

static bool load_logo_if_needed(const std::wstring& path) {
    if (!path.empty() && g_logo.path == path && !g_logo.ycc_rgba.empty() && !g_logo.dp_rgba.empty()) return true;

    // open file
    FILE* fptmp = nullptr;
    if (_wfopen_s(&fptmp, path.c_str(), L"rb") != 0 || !fptmp) return false;
    auto fp = std::unique_ptr<FILE, decltype(&fclose)>(fptmp, fclose);

    // check file size
    fseek(fp.get(), 0, SEEK_END);
    const int64_t file_size = _ftelli64(fp.get());
    fseek(fp.get(), 0, SEEK_SET);
    if (file_size < sizeof(LOGO_FILE_HEADER)) return false;

    std::vector<uint8_t> logo_data_raw(file_size, 0);
    if (fread(logo_data_raw.data(), 1, logo_data_raw.size(), fp.get()) != file_size) return false;
    fp.reset();

    const int logo_header_ver = logo_file_header_ver((const LOGO_FILE_HEADER*)logo_data_raw.data());
    if (logo_header_ver == 0) return false;

    const size_t logo_header_size = (logo_header_ver == 1) ? sizeof(LOGO_HEADER_OLD) : sizeof(LOGO_HEADER);
    const uint8_t *logo_header_ptr = logo_data_raw.data() + sizeof(LOGO_FILE_HEADER);

    LOGO_HEADER logo_header = { 0 };
    memcpy(&logo_header, logo_header_ptr, logo_header_size);
    if (logo_header_ver == 1) {
        conv_logo_header_v1_to_v2(&logo_header);
    }

    const int logo_w = logo_header.w;
    const int logo_h = logo_header.h;
    std::vector<uint8_t> ycc(logo_w * logo_h * 4, 0);
    std::vector<uint8_t> dpr(logo_w * logo_h * 4, 0);
    LOGO_PIXEL *logo_pix = (LOGO_PIXEL *)(logo_header_ptr + logo_header_size);
    size_t pos = 0;
    for (int yy = 0; yy < logo_h; yy++) {
        for (int xx = 0; xx < logo_w; xx++) {
            const auto dp_y = logo_pix->dp_y;
            const auto yv   = logo_pix->y;
            const auto dp_cb= logo_pix->dp_cb;
            const auto cb   = logo_pix->cb;
            const auto dp_cr= logo_pix->dp_cr;
            const auto cr   = logo_pix->cr;
            logo_pix++;

            // Y: 0..4096 -> 0..255
            double Y = std::min(std::max(yv * (1.0 / 4096.0), 0.0), 1.0);
            // Cb/Cr: -2048..2048 -> (-0.5..0.5) then 0..255 via +0.5 offset
            double U = std::min(std::max(cb * (1.0 / 4096.0), -0.5), 0.5);
            double V = std::min(std::max(cr * (1.0 / 4096.0), -0.5), 0.5);

            const int idx = (yy * logo_w + xx) * 4;
            ycc[idx + 0] = clamp_u8(Y * 255.0);
            ycc[idx + 1] = clamp_u8((U + 0.5) * 255.0);
            ycc[idx + 2] = clamp_u8((V + 0.5) * 255.0);
            ycc[idx + 3] = 255;
            dpr[idx + 0] = clamp_u8(dp_y  * (255.0 / LOGO_MAX_DP));
            dpr[idx + 1] = clamp_u8(dp_cb * (255.0 / LOGO_MAX_DP));
            dpr[idx + 2] = clamp_u8(dp_cr * (255.0 / LOGO_MAX_DP));
            dpr[idx + 3] = 255;
        }
    }

    g_logo.path = path;
    g_logo.logo_header = logo_header;
    g_logo.ycc_rgba = std::move(ycc);
    g_logo.dp_rgba  = std::move(dpr);
    return true;
}

// helpers to fetch string from param 0
std::wstring get_path(SCRIPT_MODULE_PARAM* p) {
    LPCSTR s = p->get_param_string(0);
    if (!s) return g_logo.path;
    int len = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
    if (len <= 0) return std::wstring();
    std::vector<wchar_t> ws(len + 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s, -1, ws.data(), len);
    return std::wstring(ws.data());
}

static void get_logo_ycc(SCRIPT_MODULE_PARAM* p) {
    const auto path = get_path(p);
    if (path.empty() || !load_logo_if_needed(path)) { p->set_error("load failed"); return; }
    p->push_result_data((void*)g_logo.ycc_rgba.data());
}

static void get_logo_dp(SCRIPT_MODULE_PARAM* p) {
    const auto path = get_path(p);
    if (!path.empty() && g_logo.path != path) {
        if (!load_logo_if_needed(path)) { p->set_error("load failed"); return; }
    }
    if (g_logo.dp_rgba.empty()) { p->set_error("no data"); return; }
    p->push_result_data((void*)g_logo.dp_rgba.data());
}

// scalar getters
static void get_x(SCRIPT_MODULE_PARAM* p) { p->push_result_int(g_logo.logo_header.x); }
static void get_y(SCRIPT_MODULE_PARAM* p) { p->push_result_int(g_logo.logo_header.y); }
static void get_w(SCRIPT_MODULE_PARAM* p) { p->push_result_int(g_logo.logo_header.w); }
static void get_h(SCRIPT_MODULE_PARAM* p) { p->push_result_int(g_logo.logo_header.h); }
static void get_fi(SCRIPT_MODULE_PARAM* p) { p->push_result_int(g_logo.logo_header.fi); }
static void get_fo(SCRIPT_MODULE_PARAM* p) { p->push_result_int(g_logo.logo_header.fo); }
static void get_st(SCRIPT_MODULE_PARAM* p) { p->push_result_int(g_logo.logo_header.st); }
static void get_ed(SCRIPT_MODULE_PARAM* p) { p->push_result_int(g_logo.logo_header.ed); }

static SCRIPT_MODULE_FUNCTION functions[] = {
    {L"get_logo_ycc", get_logo_ycc},
    {L"get_logo_dp",  get_logo_dp},
    {L"get_x", get_x}, {L"get_y", get_y}, {L"get_w", get_w}, {L"get_h", get_h},
    {L"get_fi", get_fi}, {L"get_fo", get_fo}, {L"get_st", get_st}, {L"get_ed", get_ed},
    {nullptr, nullptr}
};

static SCRIPT_MODULE_TABLE script_module_table = {L"delogo", functions};

EXTERN_C __declspec(dllexport) SCRIPT_MODULE_TABLE* GetScriptModuleTable(void) {
    return &script_module_table;
}

