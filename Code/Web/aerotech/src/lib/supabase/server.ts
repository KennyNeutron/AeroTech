import { cookies } from "next/headers";
import { createServerComponentClient } from "@supabase/auth-helpers-nextjs";

export const createServer = async () => {
  //Next.js 15: must await cookies()
  const cookieStore = await cookies();
  return createServerComponentClient({ cookies: () => cookieStore });
};
