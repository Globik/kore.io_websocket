#ifndef __H_KORE_ASSETS_H
#define __H_KORE_ASSETS_H
extern const u_int8_t asset_index_html[];
extern const u_int32_t asset_len_index_html;
extern const time_t asset_mtime_index_html;
extern const char *asset_sha256_index_html;
int asset_serve_index_html(struct http_request *);

#endif
