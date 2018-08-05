#include <kore/kore.h>
#include <kore/http.h>

int		page(struct http_request *);
int		login(struct http_request*);
int dashboard(struct http_request*);
int	redirect(struct http_request *);

int	auth_login(struct http_request *);
int	auth_user_exists(struct http_request *, char *);
void	auth_session_add(struct kore_msg *, const void *);
void	auth_session_del(struct kore_msg *, const void *);
int	auth_session(struct http_request *, const char *);

int
page(struct http_request *req)
{
	http_response(req, 200, NULL, 0);
	return (KORE_RESULT_OK);
}
