import { cookies } from "next/headers";
import { createServerComponentClient } from "@supabase/auth-helpers-nextjs";

// Return a function that returns the cookie store
export const createServer = () => {
  return createServerComponentClient({
    cookies: () => cookies(), //  not `cookies`, not `await cookies()`
  });
};
