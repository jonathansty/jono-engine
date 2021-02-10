#include <vector>

class base_meta_type {
public:
	virtual void get_name() const = 0;

	virtual void const is_struct() const = 0;
	virtual void const is_class() const = 0;

	virtual std::vector<const char*> const members() const = 0;
};
