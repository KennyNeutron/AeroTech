import { cookies } from "next/headers";
import { createServerComponentClient } from "@supabase/auth-helpers-nextjs";

export const createServer = async () => {
  // Next.js 15: cookies() is async
  const cookieStore = await cookies();

  // auth-helpers expects cookies: () => Promise<ReadonlyRequestCookies>
  return createServerComponentClient({
    cookies: async () => cookieStore, // make it async
  });
};
