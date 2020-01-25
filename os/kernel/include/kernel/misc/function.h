#pragma once
#define F_PTR_NOA(ret) ret(*)()
#define F_PTR_A1(ret,a) ret(*)(a)
#define F_PTR_A2(ret,a1,a2) ret(*)(a1,a2)
#define F_PTR_A3(ret,a1,a2,a3) ret(*)(a1,a2,a3)
#define F_PTR_A4(ret,a1,a2,a3,a4) ret(*)(a1,a2,a3,a4)
#define F_PTR_A5(ret,a1,a2,a3,a4,a5) ret(*)(a1,a2,a3,a4,a5)
