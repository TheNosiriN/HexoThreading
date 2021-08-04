#pragma once


#ifndef HEXO_HXRC_H
#define HEXO_HXRC_H


typedef uint8_t HXRC;




enum : HXRC {
	HXRC_OK,
	HXRC_INFO,
	HXRC_FATAL,
	HXRC_WARNING,
};


struct HXRC_STATE {
	const char* ErrorString = nullptr;
	HXRC Code = 0;
};


struct HXRC_APP {
	void (*func)(HXRC_STATE);
};


struct HXRC_SYS {
	std::vector<HXRC_APP> Apps;
	inline size_t AddApp(void (*func)(HXRC_STATE)){
		Apps.push_back(HXRC_APP{func});
		return Apps.size();
	}

	static inline HXRC_SYS* Get(){
		static HXRC_SYS h;
		return &h;
	}
	static inline HXRC_APP GetApp(size_t i){
		return HXRC_SYS::Get()->Apps.at(i-1);
	}
};


static inline size_t HXRC_REGISTER_APP(void (*func)(HXRC_STATE)){
	return HXRC_SYS::Get()->AddApp(func);
}




#define HXRC_BUILD(app, string, code) {                  \
	HXRC c=code;                                           \
	if (app < 1){ return c; }                              \
	HXRC_SYS::GetApp(app).func(HXRC_STATE{string, c});     \
	return c;                                              \
}

#define HXRC_BUILD_NO_RETURN(app, string) {              				\
	if (app >= 1){                            						      	\
		HXRC_SYS::GetApp(app).func(HXRC_STATE{string, HXRC_INFO});  \
	}                                                      				\
}


#define HXRC_FAILED(x) (x != HXRC_OK)
#define HXRC_SUCCESS(x) (x == HXRC_OK)

#define HXRC_PRINT(string, app) HXRC_BUILD_NO_RETURN(app, string);

#define HXRC_RETURN(x) { HXRC c=x; if (c != HXRC_OK)return c; }
#define HXRC_RETURN_INFO(string, app) HXRC_BUILD(app, string, HXRC_INFO);
#define HXRC_RETURN_FATAL(string, app) HXRC_BUILD(app, string, HXRC_FATAL);
#define HXRC_RETURN_WARNING(string, app) HXRC_BUILD(app, string, HXRC_WARNING);

#define HXRC_ASSERT(x, string, app, code) if (!(x))HXRC_BUILD(app, string, code);
#define HXRC_ASSERT_INFO(x, string, app) if (!(x))HXRC_BUILD(app, string, HXRC_INFO);
#define HXRC_ASSERT_FATAL(x, string, app) if (!(x))HXRC_BUILD(app, string, HXRC_FATAL);
#define HXRC_ASSERT_WARNING(x, string, app) if (!(x))HXRC_BUILD(app, string, HXRC_WARNING);






#endif /* end of include guard: HEXO_HXRC_H */
