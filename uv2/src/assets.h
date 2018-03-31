#ifndef __H_KORE_ASSETS_H
#define __H_KORE_ASSETS_H
extern const u_int8_t asset_front2_html[];
extern const u_int32_t asset_len_front2_html;
extern const time_t asset_mtime_front2_html;
extern const char *asset_sha256_front2_html;
int asset_serve_front2_html(struct http_request *);
extern const u_int8_t asset_frontend_html[];
extern const u_int32_t asset_len_frontend_html;
extern const time_t asset_mtime_frontend_html;
extern const char *asset_sha256_frontend_html;
int asset_serve_frontend_html(struct http_request *);

#endif
