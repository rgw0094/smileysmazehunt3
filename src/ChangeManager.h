#include <list>

struct Change {
	int x,int y;
	int area;
};

class ChangeManager {

public:

	ChangeManager();
	~ChangeManager();

	void change(int area, int x, int y);
	bool isChanged(int area, int x, int y);
	void reset();
	char* toString();

private:
	std::list<Change> theChanges;
	void addChange(int area, int x, int y);
	bool removeChange(int area, int x, int y);
};