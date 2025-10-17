import { NextResponse } from "next/server";
import type { NextRequest } from "next/server";

// Add protected route prefixes here:
const PROTECTED_PREFIXES = ["/dashboard"];

export function middleware(req: NextRequest) {
  const { pathname, searchParams } = req.nextUrl;

  // 🔐 Replace with real auth check later (e.g., Supabase helpers).
  // For Supabase Auth, presence of "sb-access-token" cookie implies a session.
  const isLoggedIn = Boolean(req.cookies.get("sb-access-token")?.value);

  // Smart redirect for "/"
  if (pathname === "/") {
    const url = req.nextUrl.clone();
    url.pathname = isLoggedIn ? "/dashboard" : "/login";
    return NextResponse.redirect(url);
  }

  // Protect certain routes
  const isProtected = PROTECTED_PREFIXES.some((p) => pathname.startsWith(p));
  if (isProtected && !isLoggedIn) {
    const url = req.nextUrl.clone();
    url.pathname = "/login";
    // Optional: remember where to go after login
    url.searchParams.set(
      "redirectTo",
      pathname + (searchParams?.toString() ? `?${searchParams}` : "")
    );
    return NextResponse.redirect(url);
  }

  return NextResponse.next();
}

// Match "/" and any protected segments
export const config = {
  matcher: ["/", "/dashboard/:path*"],
};
