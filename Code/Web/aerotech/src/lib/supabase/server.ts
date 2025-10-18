import { cookies } from "next/headers";
import { createServerComponentClient } from "@supabase/auth-helpers-nextjs";

export const createServer = async () => {
  const cookieStore = await cookies(); // Next.js 15: async cookies()

  return createServerComponentClient({
    // MUST return a Promise
    cookies: async () => cookieStore, // or: cookies: () => Promise.resolve(cookieStore)
  });
};
